// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2022, Fujitsu */

/*
 * mocks-rpma-srq_cfg.c -- librpma srq_cfg.c module mocks
 */

#include <librpma.h>

#include "cmocka_headers.h"
#include "mocks-rpma-srq_cfg.h"
#include "test-common.h"

/*
 * rpma_srq_cfg_default -- rpma_srq_cfg_default() mock
 */
struct rpma_srq_cfg *
rpma_srq_cfg_default()
{
	return MOCK_SRQ_CFG_DEFAULT;
}

/*
 * rpma_srq_cfg_get_rcqe -- rpma_srq_cfg_get_rcqe() mock
 */
void
rpma_srq_cfg_get_rcqe(const struct rpma_srq_cfg *cfg, int *rcqe)
{
	struct srq_cfg_get_mock_args *args = mock_type(struct srq_cfg_get_mock_args *);

	assert_ptr_equal(cfg, args->cfg);
	assert_non_null(rcqe);
	*rcqe = (int)args->rcq_size;
}

/*
 * rpma_srq_cfg_get_rq_size -- rpma_srq_cfg_get_rq_size() mock
 */
int
rpma_srq_cfg_get_rq_size(const struct rpma_srq_cfg *cfg, uint32_t *rq_size)
{
	struct srq_cfg_get_mock_args *args = mock_type(struct srq_cfg_get_mock_args *);

	assert_ptr_equal(cfg, args->cfg);
	assert_non_null(rq_size);

	*rq_size = args->rq_size;

	return 0;
}
