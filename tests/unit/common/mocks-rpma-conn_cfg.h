/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright 2020, Intel Corporation */

/*
 * mocks-rpma-conn_cfg.h -- a rpma-conn_cfg mocks header
 */

#ifndef MOCKS_RPMA_CONN_CFG_H
#define MOCKS_RPMA_CONN_CFG_H

/* random values */
#define MOCK_CONN_CFG_DEFAULT	(struct rpma_conn_cfg *)0xCF6D
#define MOCK_CONN_CFG_CUSTOM	(struct rpma_conn_cfg *)0xCF6C

#define MOCK_CQ_SIZE 10
#define MOCK_SQ_SIZE 11
#define MOCK_RQ_SIZE 12

/* current hardcoded values */
#define RPMA_MAX_SGE		1
#define RPMA_MAX_INLINE_DATA	0

#endif /* MOCKS_RPMA_CONN_CFG_H */
