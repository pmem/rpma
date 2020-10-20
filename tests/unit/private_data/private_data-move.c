// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020, Intel Corporation */

/*
 * private_data-copy.c -- the rpma_private_data_move() unit tests
 *
 * API covered:
 * -rpma_private_data_move()
 */

#include <rdma/rdma_cma.h>
#include <stdlib.h>

#include "cmocka_headers.h"
#include "private_data.h"
#include "private_data-common.h"
#include "librpma.h"

/*
 * move__ptr_NULL_len_0 - ptr == NULL and len == 0 should prevent setting
 * a private data
 */
static void
move__ptr_NULL_len_0(void **unused)
{
	struct rpma_conn_private_data src = {NULL, 0};
	struct rpma_conn_private_data dst = {0};
	int ret = rpma_private_data_move(&dst, &src);

	/* verify the result */
	assert_int_equal(ret, SUCCESS);
	assert_ptr_equal(src.ptr, NULL);
	assert_int_equal(src.len, 0);
	assert_ptr_equal(dst.ptr, NULL);
	assert_int_equal(dst.len, 0);
}

/*
 * move__success - happy day scenario
 */
static void
move__success(void **pdata_ptr)
{
	struct rpma_conn_private_data *src = *pdata_ptr;
	void *src_ptr = src->ptr;
	uint8_t src_len = src->len;

	/* run test */
	struct rpma_conn_private_data dst = {0};
	int ret = rpma_private_data_move(&dst, src);

	/* verify the result */
	assert_int_equal(ret, SUCCESS);
	assert_ptr_equal(src->ptr, NULL);
	assert_int_equal(src->len, 0);
	assert_int_equal(dst.len, src_len);
	assert_string_equal(dst.ptr, src_ptr);

	/* cleanup */
	rpma_private_data_discard(&dst);
	assert_int_equal(ret, SUCCESS);
	assert_ptr_equal(dst.ptr, NULL);
	assert_int_equal(dst.len, 0);
}

static const struct CMUnitTest test_move[] = {
	/* rpma_private_data_move() unit tests */
	cmocka_unit_test(move__ptr_NULL_len_0),
	cmocka_unit_test_setup_teardown(move__success,
		setup__private_data, teardown__private_data),
	cmocka_unit_test(NULL)
};

int
main(int argc, char *argv[])
{
	return cmocka_run_group_tests(test_move, NULL, NULL);
}
