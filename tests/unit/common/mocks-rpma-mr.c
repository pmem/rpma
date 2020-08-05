// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020, Intel Corporation */

/*
 * mocks-rpma-mr.c -- librpma mr.c module mocks
 */

#include <rdma/rdma_cma.h>
#include <librpma.h>

#include "cmocka_headers.h"
#include "mr.h"
#include "rpma_err.h"
#include "test-common.h"

/*
 * rpma_mr_read -- rpma_mr_read() mock
 */
int
rpma_mr_read(struct ibv_qp *qp,
	struct rpma_mr_local *dst, size_t dst_offset,
	struct rpma_mr_remote *src,  size_t src_offset,
	size_t len, int flags, void *op_context)
{
	assert_non_null(qp);
	assert_non_null(dst);
	assert_non_null(src);
	assert_int_not_equal(flags, 0);

	check_expected_ptr(qp);
	check_expected_ptr(dst);
	check_expected(dst_offset);
	check_expected_ptr(src);
	check_expected(src_offset);
	check_expected(len);
	check_expected(flags);
	check_expected_ptr(op_context);

	return mock_type(int);
}

/*
 * rpma_mr_reg -- a mock of rpma_mr_reg()
 */
int
rpma_mr_reg(struct rpma_peer *peer, void *ptr, size_t size, int usage,
		enum rpma_mr_plt plt, struct rpma_mr_local **mr_ptr)
{
	check_expected_ptr(peer);
	check_expected(size);
	check_expected(usage);
	assert_non_null(mr_ptr);

	void **paddr = mock_type(void **);
	assert_ptr_equal(ptr, *paddr);

	*mr_ptr = mock_type(struct rpma_mr_local *);
	if (*mr_ptr == NULL)
		return mock_type(int); /* errno */

	return 0;
}

/*
 * rpma_mr_dereg -- a mock of rpma_mr_dereg()
 */
int
rpma_mr_dereg(struct rpma_mr_local **mr_ptr)
{
	assert_non_null(mr_ptr);
	check_expected_ptr(*mr_ptr);

	*mr_ptr = NULL;
	return 0;
}
