/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright 2022, Fujitsu */

/*
 * mocks-rpma-srq_cfg.h -- a rpma-srq_cfg mocks header
 */

#include <stdbool.h>

#ifndef MOCKS_RPMA_SRQ_CFG_H
#define MOCKS_RPMA_SRQ_CFG_H

/* random values */
#define MOCK_SRQ_CFG_DEFAULT	(struct rpma_srq_cfg *)0xCF6E
#define MOCK_SRQ_CFG_CUSTOM	(struct rpma_srq_cfg *)0xCF6F

#define MOCK_SRQ_SIZE_DEFAULT		100
#define MOCK_SRQ_SIZE_CUSTOM		200
#define MOCK_SRQ_RCQ_SIZE_DEFAULT	100
#define MOCK_SRQ_RCQ_SIZE_CUSTOM	0

struct srq_cfg_get_mock_args {
	struct rpma_srq_cfg *cfg;
	uint32_t rq_size;
	uint32_t rcq_size;
};

#endif /* MOCKS_RPMA_SRQ_CFG_H */
