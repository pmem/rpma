/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright 2020, Intel Corporation */

#include <stdbool.h>
/*
 * mocks-rpma-conn_cfg.h -- a rpma-conn_cfg mocks header
 */

#ifndef MOCKS_RPMA_CONN_CFG_H
#define MOCKS_RPMA_CONN_CFG_H

/* random values */
#define MOCK_CONN_CFG_DEFAULT	(struct rpma_conn_cfg *)0xCF6D
#define MOCK_CONN_CFG_CUSTOM	(struct rpma_conn_cfg *)0xCF6C

#define MOCK_CQ_SIZE_DEFAULT 10
#define MOCK_SQ_SIZE_DEFAULT 11
#define MOCK_RQ_SIZE_DEFAULT 12

#define MOCK_TIMEOUT_MS_CUSTOM	4034
#define MOCK_CQ_SIZE_CUSTOM	13
#define MOCK_SQ_SIZE_CUSTOM	14
#define MOCK_RQ_SIZE_CUSTOM	15
#define MOCK_USE_SRQ_CUSTOM	true
#define MOCK_DONTUSE_SRQ_CUSTOM	false
#define MOCK_SRQ_WR_SIZE_CUSTOM	16
#define MOCK_SRQ_SGE_SIZE_CUSTOM 17
#define MOCK_SRQ_LIMIT_CUSTOM	18

struct conn_cfg_get_timeout_mock_args {
	struct rpma_conn_cfg *cfg;
	int timeout_ms;
};

struct conn_cfg_get_q_size_mock_args {
	struct rpma_conn_cfg *cfg;
	uint32_t q_size;
};

struct conn_cfg_get_use_srq_mock_args {
	struct rpma_conn_cfg *cfg;
	bool use_srq;
};

struct conn_cfg_get_max_wr_mock_args {
	struct rpma_conn_cfg *cfg;
	uint32_t max_wr;
};

struct conn_cfg_get_max_sge_mock_args {
	struct rpma_conn_cfg *cfg;
	uint32_t max_sge;
};

struct conn_cfg_get_srq_limit_mock_args {
	struct rpma_conn_cfg *cfg;
	uint32_t srq_limit;
};
/* current hardcoded values */
#define RPMA_MAX_SGE		1
#define RPMA_MAX_INLINE_DATA	0

#endif /* MOCKS_RPMA_CONN_CFG_H */
