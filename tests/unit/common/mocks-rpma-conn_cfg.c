// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020, Intel Corporation */
/* Copyright 2021, Fujitsu */

/*
 * mocks-rpma-conn_cfg.c -- librpma conn_cfg.c module mocks
 */

#include <librpma.h>

#include "cmocka_headers.h"
#include "conn_cfg.h"
#include "mocks-rpma-conn_cfg.h"
#include "test-common.h"

/*
 * rpma_conn_cfg_default -- rpma_conn_cfg_default() mock
 */
struct rpma_conn_cfg *
rpma_conn_cfg_default()
{
	return MOCK_CONN_CFG_DEFAULT;
}

/*
 * rpma_conn_cfg_get_timeout -- rpma_conn_cfg_get_timeout() mock
 */
int
rpma_conn_cfg_get_timeout(const struct rpma_conn_cfg *cfg, int *timeout_ms)
{
	struct conn_cfg_get_timeout_mock_args *args =
			mock_type(struct conn_cfg_get_timeout_mock_args *);

	assert_ptr_equal(cfg, args->cfg);
	assert_non_null(timeout_ms);

	*timeout_ms = args->timeout_ms;

	return 0;
}

/*
 * rpma_conn_cfg_get_cqe -- rpma_conn_cfg_get_cqe() mock
 */
int
rpma_conn_cfg_get_cqe(const struct rpma_conn_cfg *cfg, int *cqe)
{
	struct conn_cfg_get_cq_size_mock_args *args =
			mock_type(struct conn_cfg_get_cq_size_mock_args *);

	assert_ptr_equal(cfg, args->cfg);
	assert_non_null(cqe);

	*cqe = (int)args->cq_size;

	return 0;
}

/*
 * rpma_conn_cfg_get_rcqe -- rpma_conn_cfg_get_rcqe() mock
 */
int
rpma_conn_cfg_get_rcqe(const struct rpma_conn_cfg *cfg, int *rcqe)
{
	struct conn_cfg_get_cq_size_mock_args *args =
			mock_type(struct conn_cfg_get_cq_size_mock_args *);

	assert_ptr_equal(cfg, args->cfg);
	assert_non_null(rcqe);

	*rcqe = (int)args->rcq_size;

	return 0;
}

/*
 * rpma_conn_cfg_get_sq_size -- rpma_conn_cfg_get_sq_size() mock
 */
int
rpma_conn_cfg_get_sq_size(const struct rpma_conn_cfg *cfg, uint32_t *sq_size)
{
	struct conn_cfg_get_q_size_mock_args *args =
			mock_type(struct conn_cfg_get_q_size_mock_args *);

	assert_ptr_equal(cfg, args->cfg);
	assert_non_null(sq_size);

	*sq_size = args->q_size;

	return 0;
}

/*
 * rpma_conn_cfg_get_rq_size -- rpma_conn_cfg_get_rq_size() mock
 */
int
rpma_conn_cfg_get_rq_size(const struct rpma_conn_cfg *cfg, uint32_t *rq_size)
{
	struct conn_cfg_get_q_size_mock_args *args =
			mock_type(struct conn_cfg_get_q_size_mock_args *);

	assert_ptr_equal(cfg, args->cfg);
	assert_non_null(rq_size);

	*rq_size = args->q_size;

	return 0;
}
