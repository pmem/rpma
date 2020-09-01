// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020, Intel Corporation */

/*
 * conn_cfg.c -- librpma connection-configuration-related implementations
 */

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

struct rpma_conn_cfg {
	int timeout_ms;	/* connection establishment timeout */
	int cq_size;	/* CQ size */
	int sq_size;	/* SQ size */
	int rq_size;	/* RQ size */
};

static struct rpma_conn_cfg Conn_cfg_default  = {
	.timeout_ms = RPMA_DEFAULT_TIMEOUT_MS,
	.cq_size = RPMA_DEFAULT_Q_SIZE,
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
rpma_conn_cfg_get_timeout(struct rpma_conn_cfg *cfg, int *timeout_ms)
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
rpma_conn_cfg_set_cq_size(struct rpma_conn_cfg *cfg, int cq_size)
{
	if (cfg == NULL || cq_size < 0)
		return RPMA_E_INVAL;

	cfg->cq_size = cq_size;

	return 0;
}

/*
 * rpma_conn_cfg_get_cq_size -- get CQ size for the connection
 */
int
rpma_conn_cfg_get_cq_size(struct rpma_conn_cfg *cfg, int *cq_size)
{
	if (cfg == NULL || cq_size == NULL)
		return RPMA_E_INVAL;

	*cq_size = cfg->cq_size;

	return 0;
}

/*
 * rpma_conn_cfg_set_sq_size -- set SQ size for the connection
 */
int
rpma_conn_cfg_set_sq_size(struct rpma_conn_cfg *cfg, int sq_size)
{
	if (cfg == NULL || sq_size < 0)
		return RPMA_E_INVAL;

	cfg->sq_size = sq_size;

	return 0;
}

/*
 * rpma_conn_cfg_get_sq_size -- get SQ size for the connection
 */
int
rpma_conn_cfg_get_sq_size(struct rpma_conn_cfg *cfg, int *sq_size)
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
rpma_conn_cfg_set_rq_size(struct rpma_conn_cfg *cfg, int rq_size)
{
	if (cfg == NULL || rq_size < 0)
		return RPMA_E_INVAL;

	cfg->rq_size = rq_size;

	return 0;
}

/*
 * rpma_conn_cfg_get_rq_size -- get RQ size for the connection
 */
int
rpma_conn_cfg_get_rq_size(struct rpma_conn_cfg *cfg, int *rq_size)
{
	if (cfg == NULL || rq_size == NULL)
		return RPMA_E_INVAL;

	*rq_size = cfg->rq_size;

	return 0;
}
