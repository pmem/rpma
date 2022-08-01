// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020-2022, Intel Corporation */

/*
 * mocks-rpma-flush.c -- librpma flush.c module mocks
 */

#include <rdma/rdma_cma.h>
#include <librpma.h>

#include "cmocka_headers.h"
#include "flush.h"
#include "mocks-rpma-flush.h"
#include "test-common.h"

struct rpma_flush Rpma_flush;

/*
 * rpma_flush_mock_execute -- rpma_flush_apm_execute() mock
 */
int
rpma_flush_mock_execute(struct ibv_qp *qp, struct rpma_flush *flush,
	struct rpma_mr_remote *dst, size_t dst_offset, size_t len,
	enum rpma_flush_type type, int flags, const void *op_context)
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
rpma_flush_new(struct rpma_peer *peer, struct rpma_flush **flush_ptr)
{
	assert_int_equal(peer, MOCK_PEER);
	assert_non_null(flush_ptr);
	Rpma_flush.func = rpma_flush_mock_execute;

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
	/* XXX validate the errno handling */
	if (ret == RPMA_E_PROVIDER)
		errno = mock_type(int);

	return ret;
}
