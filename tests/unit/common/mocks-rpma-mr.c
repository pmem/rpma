// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020-2022, Intel Corporation */

/*
 * mocks-rpma-mr.c -- librpma mr.c module mocks
 */

#include <rdma/rdma_cma.h>
#include <librpma.h>

#include "cmocka_headers.h"
#include "mr.h"
#include "test-common.h"

/*
 * rpma_mr_read -- rpma_mr_read() mock
 */
int
rpma_mr_read(struct ibv_qp *qp,
	struct rpma_mr_local *dst, size_t dst_offset,
	const struct rpma_mr_remote *src,  size_t src_offset,
	size_t len, int flags, const void *op_context)
{
	assert_non_null(qp);
	assert_int_not_equal(flags, 0);
	assert_true((src != NULL && dst != NULL) ||
		(src == NULL && dst == NULL &&
		dst_offset == 0 && src_offset == 0 && len == 0));

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
 * rpma_mr_write -- rpma_mr_write() mock
 */
int
rpma_mr_write(struct ibv_qp *qp,
	struct rpma_mr_remote *dst, size_t dst_offset,
	const struct rpma_mr_local *src,  size_t src_offset,
	size_t len, int flags, enum ibv_wr_opcode operation,
	uint32_t imm, const void *op_context)
{
	assert_non_null(qp);
	assert_int_not_equal(flags, 0);
	assert_true((src != NULL && dst != NULL) ||
		(src == NULL && dst == NULL &&
		dst_offset == 0 && src_offset == 0 && len == 0));

	check_expected_ptr(qp);
	check_expected_ptr(dst);
	check_expected(dst_offset);
	check_expected_ptr(src);
	check_expected(src_offset);
	check_expected(len);
	check_expected(flags);
	check_expected(operation);
	check_expected(imm);
	check_expected_ptr(op_context);

	return mock_type(int);
}

/*
 * rpma_mr_atomic_write -- rpma_mr_atomic_write() mock
 */
int
rpma_mr_atomic_write(struct ibv_qp *qp,
	struct rpma_mr_remote *dst, size_t dst_offset,
	const char src[8], int flags, const void *op_context)
{
	assert_non_null(qp);
	assert_int_not_equal(flags, 0);
	assert_non_null(src);
	assert_non_null(dst);

	check_expected_ptr(qp);
	check_expected_ptr(dst);
	check_expected(dst_offset);
	check_expected_ptr(src);
	check_expected(flags);
	check_expected_ptr(op_context);

	return mock_type(int);
}

/*
 * rpma_mr_reg -- a mock of rpma_mr_reg()
 */
int
rpma_mr_reg(struct rpma_peer *peer, void *ptr, size_t size, int usage,
	struct rpma_mr_local **mr_ptr)
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

	int ret = mock_type(int);
	/* XXX validate the errno handling */
	if (ret == RPMA_E_PROVIDER)
		errno = mock_type(int);

	*mr_ptr = NULL;

	return ret;
}

/*
 * rpma_mr_send -- mock of rpma_mr_send
 */
int
rpma_mr_send(struct ibv_qp *qp,
	const struct rpma_mr_local *src,  size_t offset,
	size_t len, int flags, enum ibv_wr_opcode operation,
	uint32_t imm, const void *op_context)
{
	assert_non_null(qp);
	assert_int_not_equal(flags, 0);
	assert_true(src != NULL || (offset == 0 && len == 0));

	check_expected_ptr(qp);
	check_expected_ptr(src);
	check_expected(offset);
	check_expected(len);
	check_expected(flags);
	check_expected(operation);
	check_expected(imm);
	check_expected_ptr(op_context);

	return mock_type(int);
}

/*
 * rpma_mr_recv -- mock of rpma_mr_recv
 */
int
rpma_mr_recv(struct ibv_qp *qp,
	struct rpma_mr_local *dst,  size_t offset,
	size_t len, const void *op_context)
{
	assert_non_null(qp);
	assert_true(dst != NULL || (offset == 0 && len == 0));

	check_expected_ptr(qp);
	check_expected_ptr(dst);
	check_expected(offset);
	check_expected(len);
	check_expected_ptr(op_context);

	return mock_type(int);
}

/*
 * rpma_mr_remote_get_flush_type -- mock of rpma_mr_remote_get_flush_type
 */
int
rpma_mr_remote_get_flush_type(const struct rpma_mr_remote *mr, int *flush_type)
{
	check_expected_ptr(mr);
	assert_non_null(flush_type);

	*flush_type = mock_type(int);

	return 0;
}
