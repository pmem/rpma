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
 *
 * XXX allocate, initialize to defaults and return
 */
int
rpma_conn_cfg_new(struct rpma_conn_cfg **cfg_ptr)
{
	return RPMA_E_NOSUPP;
}

/*
 * rpma_conn_cfg_delete -- delete the connection configuration object
 *
 * XXX free and set user's pointer to NULL
 */
int
rpma_conn_cfg_delete(struct rpma_conn_cfg **cfg_ptr)
{
	return RPMA_E_NOSUPP;
}

/*
 * rpma_conn_cfg_set_timeout -- set connection establishment timeout
 */
int
rpma_conn_cfg_set_timeout(struct rpma_conn_cfg *cfg, int timeout_ms)
{
	return RPMA_E_NOSUPP;
}

/*
 * rpma_conn_cfg_get_timeout -- get connection establishment timeout
 */
int
rpma_conn_cfg_get_timeout(struct rpma_conn_cfg *cfg, int *timeout_ms)
{
	return RPMA_E_NOSUPP;
}

/*
 * rpma_conn_cfg_set_cq_size -- set CQ size for the connection
 */
int
rpma_conn_cfg_set_cq_size(struct rpma_conn_cfg *cfg, int cq_size)
{
	return RPMA_E_NOSUPP;
}

/*
 * rpma_conn_cfg_get_cq_size -- get CQ size for the connection
 */
int
rpma_conn_cfg_get_cq_size(struct rpma_conn_cfg *cfg, int *cq_size)
{
	return RPMA_E_NOSUPP;
}

/*
 * rpma_conn_cfg_set_sq_size -- set SQ size for the connection
 */
int
rpma_conn_cfg_set_sq_size(struct rpma_conn_cfg *cfg, int sq_size)
{
	return RPMA_E_NOSUPP;
}

/*
 * rpma_conn_cfg_get_sq_size -- get SQ size for the connection
 */
int
rpma_conn_cfg_get_sq_size(struct rpma_conn_cfg *cfg, int *sq_size)
{
	return RPMA_E_NOSUPP;
}

/*
 * rpma_conn_cfg_set_rq_size -- set RQ size for the connection
 */
int
rpma_conn_cfg_set_rq_size(struct rpma_conn_cfg *cfg, int rq_size)
{
	return RPMA_E_NOSUPP;
}

/*
 * rpma_conn_cfg_get_rq_size -- get RQ size for the connection
 */
int
rpma_conn_cfg_get_rq_size(struct rpma_conn_cfg *cfg, int *rq_size)
{
	return RPMA_E_NOSUPP;
}
