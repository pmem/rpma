/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright 2020, Intel Corporation */

/*
 * conn_cfg.h -- librpma connection-configuration-related internal definitions
 */

#ifndef LIBRPMA_CONN_CFG_H
#define LIBRPMA_CONN_CFG_H

#include "librpma.h"

/* the maximum number of scatter/gather elements in any Work Request */
#define RPMA_MAX_SGE 1
struct rpma_conn_cfg {
	bool use_srq;
	int timeout_ms;	/* connection establishment timeout */
	uint32_t cq_size;	/* CQ size */
	uint32_t sq_size;	/* SQ size */
	uint32_t rq_size;	/* RQ size */
	/* SRQ only attributes */
	uint32_t max_wr;	/* maximum number of WRs in the SRQ */
	uint32_t max_sge;	/* maximum number of scatter elements per WR */
	uint32_t srq_limit;	/* the limit value of the SRQ */
};

/*
 * ERRORS
 * rpma_conn_cfg_default() cannot fail.
 */
struct rpma_conn_cfg *rpma_conn_cfg_default();

/*
 * ERRORS
 * rpma_conn_cfg_get_cqe() can fail with the following error:
 *
 * - RPMA_E_INVAL - cfg or cqe is NULL
 */
int rpma_conn_cfg_get_cqe(const struct rpma_conn_cfg *cfg, int *cqe);

#endif /* LIBRPMA_CONN_CFG_H */
