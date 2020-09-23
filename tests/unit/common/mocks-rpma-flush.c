// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020, Intel Corporation */

/*
 * mocks-rpma-flush.c -- librpma flush.c module mocks
 */

#include <rdma/rdma_cma.h>
#include <librpma.h>

#include "cmocka_headers.h"
#include "flush.h"
#include "mocks-rpma-flush.h"
#include "rpma_err.h"
#include "test-common.h"

struct rpma_flush Rpma_flush;

/*
 * rpma_flush_mock_do -- rpma_flush_apm_do() mock
 */
int
rpma_flush_mock_do(struct ibv_qp *qp, struct rpma_flush *flush,
	struct rpma_mr_remote *dst, size_t dst_offset,
	size_t len, enum rpma_flush_type type, int flags, void *op_context)
{
	assert_non_null(qp);
	assert_non_null(flush);
	assert_non_null(dst);
	assert_int_not_equal(flags, 0);

	check_expected_ptr(qp);
	check_expected_ptr(flush);
	check_expected_ptr(dst);
	check_expected(dst_offset);
	check_expected(len);
	check_expected(flags);
	check_expected_ptr(op_context);

	return 0;
}

/*
 * rpma_flush_new -- rpma_flush_new() mock
 */
int
rpma_flush_new(const struct rpma_peer *peer, struct rpma_flush **flush_ptr)
{
	assert_int_equal(peer, MOCK_PEER);
	assert_non_null(flush_ptr);
	Rpma_flush.func = rpma_flush_mock_do;

	int ret = mock_type(int);
	if (ret == MOCK_OK)
		*flush_ptr = MOCK_FLUSH;

	return ret;
}

/*
 * rpma_flush_delete -- rpma_flush_delete() mock
 */
int
rpma_flush_delete(struct rpma_flush **flush_ptr)
{
	assert_ptr_equal(*flush_ptr, MOCK_FLUSH);
	*flush_ptr = NULL;

	int ret = mock_type(int);
	if (ret == RPMA_E_PROVIDER)
		Rpma_provider_error = mock_type(int);

	return ret;
}
