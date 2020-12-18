// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020, Intel Corporation */

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
	struct conn_cfg_get_q_size_mock_args *args =
			mock_type(struct conn_cfg_get_q_size_mock_args *);

	assert_ptr_equal(cfg, args->cfg);
	assert_non_null(cqe);

	*cqe = (int)args->q_size;

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

/*
 * rpma_conn_cfg_get_use_srq -- rpma_conn_cfg_get_use_srq() mock
 */
int
rpma_conn_cfg_get_use_srq(const struct rpma_conn_cfg *cfg, bool *use_srq)
{
	struct conn_cfg_get_use_srq_mock_args *args =
			mock_type(struct conn_cfg_get_use_srq_mock_args *);

	assert_ptr_equal(cfg, args->cfg);
	assert_non_null(use_srq);
	*use_srq = args->use_srq;
	return 0;
}

/*
 * rpma_conn_cfg_get_max_wr -- rpma_conn_cfg_get_max_wr() mock
 */
int
rpma_conn_cfg_get_max_wr(const struct rpma_conn_cfg *cfg, uint32_t *max_wr)
{
	struct conn_cfg_get_max_wr_mock_args *args =
			mock_type(struct conn_cfg_get_max_wr_mock_args *);

	assert_ptr_equal(cfg, args->cfg);
	assert_non_null(max_wr);
	*max_wr = args->max_wr;
	return 0;
}

/*
 * rpma_conn_cfg_get_max_sge -- rpma_conn_cfg_get_max_sge() mock
 */
int
rpma_conn_cfg_get_max_sge(const struct rpma_conn_cfg *cfg, uint32_t *max_sge)
{
	struct conn_cfg_get_max_sge_mock_args *args =
			mock_type(struct conn_cfg_get_max_sge_mock_args *);

	assert_ptr_equal(cfg, args->cfg);
	assert_non_null(max_sge);
	*max_sge = args->max_sge;
	return 0;
}

/*
 * rpma_conn_cfg_get_srq_limit -- rpma_conn_cfg_get_srq_limit() mock
 */
int
rpma_conn_cfg_get_srq_limit(const struct rpma_conn_cfg *cfg, uint32_t *srq_limit)
{
	struct conn_cfg_get_srq_limit_mock_args *args =
			mock_type(struct conn_cfg_get_srq_limit_mock_args *);

	assert_ptr_equal(cfg, args->cfg);
	assert_non_null(srq_limit);
	*srq_limit = args->srq_limit;
	return 0;
}
