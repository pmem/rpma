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
rpma_conn_cfg_get_timeout(struct rpma_conn_cfg *cfg, int *timeout_ms)
{
	check_expected_ptr(cfg);
	assert_non_null(timeout_ms);

	*timeout_ms = RPMA_DEFAULT_TIMEOUT_MS;

	return 0;
}

/*
 * rpma_conn_cfg_get_cq_size -- rpma_conn_cfg_get_cq_size() mock
 */
int
rpma_conn_cfg_get_cq_size(struct rpma_conn_cfg *cfg, uint32_t *cq_size)
{
	check_expected_ptr(cfg);
	assert_non_null(cq_size);

	*cq_size = MOCK_CQ_SIZE;

	return 0;
}

/*
 * rpma_conn_cfg_get_cqe -- rpma_conn_cfg_get_cqe() mock
 */
int
rpma_conn_cfg_get_cqe(struct rpma_conn_cfg *cfg, int *cqe)
{
	check_expected_ptr(cfg);
	assert_non_null(cqe);

	*cqe = MOCK_CQ_SIZE;

	return 0;
}

/*
 * rpma_conn_cfg_get_sq_size -- rpma_conn_cfg_get_sq_size() mock
 */
int
rpma_conn_cfg_get_sq_size(struct rpma_conn_cfg *cfg, uint32_t *sq_size)
{
	check_expected_ptr(cfg);
	assert_non_null(sq_size);

	*sq_size = MOCK_SQ_SIZE;

	return 0;
}

/*
 * rpma_conn_cfg_get_rq_size -- rpma_conn_cfg_get_rq_size() mock
 */
int
rpma_conn_cfg_get_rq_size(struct rpma_conn_cfg *cfg, uint32_t *rq_size)
{
	check_expected_ptr(cfg);
	assert_non_null(rq_size);

	*rq_size = MOCK_RQ_SIZE;

	return 0;
}
