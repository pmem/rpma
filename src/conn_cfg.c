// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020, Intel Corporation */
/* Copyright 2021, Fujitsu */

/*
 * conn_cfg.c -- librpma connection-configuration-related implementations
 */

#include <limits.h>
#include <stdlib.h>
#include <rdma/rdma_cma.h>

#include "common.h"
#include "conn_cfg.h"
#include "conn_req.h"
#include "librpma.h"
#include "log_internal.h"

#ifdef TEST_MOCK_ALLOC
#include "cmocka_alloc.h"
#endif

/*
 * For the simplicity sake, it is assumed all CQ/SQ/RQ default sizes are equal.
 */
#define RPMA_DEFAULT_Q_SIZE 10

/*
 * The default size of rcq is 0 - no separate receiving CQ
 */
#define RPMA_DEFAULT_RCQ_SIZE	0

struct rpma_conn_cfg {
	int timeout_ms;	/* connection establishment timeout */
	uint32_t cq_size;	/* CQ size */
	uint32_t rcq_size;	/* receiving CQ size */
	uint32_t sq_size;	/* SQ size */
	uint32_t rq_size;	/* RQ size */
};

static struct rpma_conn_cfg Conn_cfg_default  = {
	.timeout_ms = RPMA_DEFAULT_TIMEOUT_MS,
	.cq_size = RPMA_DEFAULT_Q_SIZE,
	.rcq_size = RPMA_DEFAULT_RCQ_SIZE,
	.sq_size = RPMA_DEFAULT_Q_SIZE,
	.rq_size = RPMA_DEFAULT_Q_SIZE
};

/* internal librpma API */

/*
 * rpma_conn_cfg_default -- return pointer to default connection configuration
 * object
 */
struct rpma_conn_cfg *
rpma_conn_cfg_default()
{
	return &Conn_cfg_default;
}

/*
 * Round down the cq_size when it is too big for storing
 * into an int type of value. Convert otherwise.
 */
static int rpma_round_down_size(uint32_t size)
{
	if (size > INT_MAX)
		return INT_MAX;

	return (int)size;
}

/*
 * rpma_conn_cfg_get_cqe -- Get cq_size and rcq_size and pass them
 * into ibv_create_cq() as the argument cqe.
 */
int
rpma_conn_cfg_get_cqe(const struct rpma_conn_cfg *cfg, int *cqe, int *recv_cqe)
{
	if (cfg == NULL || cqe == NULL || recv_cqe == NULL)
		return RPMA_E_INVAL;

	uint32_t cq_size;
	(void) rpma_conn_cfg_get_cq_size(cfg, &cq_size);

	*cqe = rpma_round_down_size(cq_size);

	uint32_t rcq_size;
	(void) rpma_conn_cfg_get_rcq_size(cfg, &rcq_size);

	*recv_cqe = rpma_round_down_size(rcq_size);

	return 0;
}

/* public librpma API */

/*
 * rpma_conn_cfg_new -- create a new connection configuration object
 */
int
rpma_conn_cfg_new(struct rpma_conn_cfg **cfg_ptr)
{
	if (cfg_ptr == NULL)
		return RPMA_E_INVAL;

	*cfg_ptr = malloc(sizeof(struct rpma_conn_cfg));
	if (*cfg_ptr == NULL)
		return RPMA_E_NOMEM;

	memcpy(*cfg_ptr, &Conn_cfg_default, sizeof(struct rpma_conn_cfg));

	return 0;
}

/*
 * rpma_conn_cfg_delete -- delete the connection configuration object
 */
int
rpma_conn_cfg_delete(struct rpma_conn_cfg **cfg_ptr)
{
	if (cfg_ptr == NULL)
		return RPMA_E_INVAL;

	if (*cfg_ptr == NULL)
		return 0;

	free(*cfg_ptr);
	*cfg_ptr = NULL;

	return 0;
}

/*
 * rpma_conn_cfg_set_timeout -- set connection establishment timeout
 */
int
rpma_conn_cfg_set_timeout(struct rpma_conn_cfg *cfg, int timeout_ms)
{
	if (cfg == NULL || timeout_ms < 0)
		return RPMA_E_INVAL;

	cfg->timeout_ms = timeout_ms;

	return 0;
}

/*
 * rpma_conn_cfg_get_timeout -- get connection establishment timeout
 */
int
rpma_conn_cfg_get_timeout(const struct rpma_conn_cfg *cfg, int *timeout_ms)
{
	if (cfg == NULL || timeout_ms == NULL)
		return RPMA_E_INVAL;

	*timeout_ms = cfg->timeout_ms;

	return 0;
}

/*
 * rpma_conn_cfg_set_cq_size -- set CQ size for the connection
 */
int
rpma_conn_cfg_set_cq_size(struct rpma_conn_cfg *cfg, uint32_t cq_size)
{
	if (cfg == NULL)
		return RPMA_E_INVAL;

	cfg->cq_size = cq_size;

	return 0;
}

/*
 * rpma_conn_cfg_get_cq_size -- get CQ size for the connection
 */
int
rpma_conn_cfg_get_cq_size(const struct rpma_conn_cfg *cfg, uint32_t *cq_size)
{
	if (cfg == NULL || cq_size == NULL)
		return RPMA_E_INVAL;

	*cq_size = cfg->cq_size;

	return 0;
}

/*
 * rpma_conn_cfg_set_rcq_size -- set receiving CQ size for the connection
 */
int
rpma_conn_cfg_set_rcq_size(struct rpma_conn_cfg *cfg, uint32_t rcq_size)
{
	if (cfg == NULL)
		return RPMA_E_INVAL;

	cfg->rcq_size = rcq_size;

	return 0;
}

/*
 * rpma_conn_cfg_get_rcq_size -- get receiving CQ size for the connection
 */
int
rpma_conn_cfg_get_rcq_size(const struct rpma_conn_cfg *cfg, uint32_t *rcq_size)
{
	if (cfg == NULL || rcq_size == NULL)
		return RPMA_E_INVAL;

	*rcq_size = cfg->rcq_size;

	return 0;
}

/*
 * rpma_conn_cfg_set_sq_size -- set SQ size for the connection
 */
int
rpma_conn_cfg_set_sq_size(struct rpma_conn_cfg *cfg, uint32_t sq_size)
{
	if (cfg == NULL)
		return RPMA_E_INVAL;

	cfg->sq_size = sq_size;

	return 0;
}

/*
 * rpma_conn_cfg_get_sq_size -- get SQ size for the connection
 */
int
rpma_conn_cfg_get_sq_size(const struct rpma_conn_cfg *cfg, uint32_t *sq_size)
{
	if (cfg == NULL || sq_size == NULL)
		return RPMA_E_INVAL;

	*sq_size = cfg->sq_size;

	return 0;
}

/*
 * rpma_conn_cfg_set_rq_size -- set RQ size for the connection
 */
int
rpma_conn_cfg_set_rq_size(struct rpma_conn_cfg *cfg, uint32_t rq_size)
{
	if (cfg == NULL)
		return RPMA_E_INVAL;

	cfg->rq_size = rq_size;

	return 0;
}

/*
 * rpma_conn_cfg_get_rq_size -- get RQ size for the connection
 */
int
rpma_conn_cfg_get_rq_size(const struct rpma_conn_cfg *cfg, uint32_t *rq_size)
{
	if (cfg == NULL || rq_size == NULL)
		return RPMA_E_INVAL;

	*rq_size = cfg->rq_size;

	return 0;
}
