/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright 2020-2022, Intel Corporation */
/* Copyright 2021-2022, Fujitsu */

/*
 * mocks-rpma-conn_cfg.h -- a rpma-conn_cfg mocks header
 */

#include <stdbool.h>

#ifndef MOCKS_RPMA_CONN_CFG_H
#define MOCKS_RPMA_CONN_CFG_H

/* random values */
#define MOCK_CONN_CFG_DEFAULT	(struct rpma_conn_cfg *)0xCF6D
#define MOCK_CONN_CFG_CUSTOM	(struct rpma_conn_cfg *)0xCF6C

#define MOCK_CQ_SIZE_DEFAULT	10
#define MOCK_RCQ_SIZE_DEFAULT	0
#define MOCK_SQ_SIZE_DEFAULT	11
#define MOCK_RQ_SIZE_DEFAULT	12
#define MOCK_SHARED_DEFAULT	false

#define MOCK_TIMEOUT_MS_CUSTOM	4034
#define MOCK_CQ_SIZE_CUSTOM	13
#define MOCK_RCQ_SIZE_CUSTOM	16
#define MOCK_SQ_SIZE_CUSTOM	14
#define MOCK_RQ_SIZE_CUSTOM	15
#define MOCK_SHARED_CUSTOM	true

struct conn_cfg_get_mock_args {
	struct rpma_conn_cfg *cfg;
	int timeout_ms;
	uint32_t sq_size;
	uint32_t rq_size;
	uint32_t cq_size;
	uint32_t rcq_size;
	bool shared;
	struct rpma_srq *srq;
	struct rpma_cq *srq_rcq;
};

/* current hardcoded values */
#define RPMA_MAX_SGE		1
#define RPMA_MAX_INLINE_DATA	8

#endif /* MOCKS_RPMA_CONN_CFG_H */
