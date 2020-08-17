// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020, Intel Corporation */

/*
 * private_data-copy.c -- the rpma_private_data_copy() unit tests
 *
 * API covered:
 * -rpma_private_data_copy()
 */

#include <rdma/rdma_cma.h>
#include <stdlib.h>

#include "cmocka_headers.h"
#include "private_data.h"
#include "private_data-common.h"
#include "librpma.h"

/*
 * copy__ptr_NULL_len_0 - ptr == NULL and len == 0 should prevent storing
 * a private data
 */
static void
copy__ptr_NULL_len_0(void **unused)
{
	struct rpma_conn_private_data src = {NULL, 0};
	struct rpma_conn_private_data dst = {0};
	int ret = rpma_private_data_copy(&dst, &src);

	/* verify the result */
	assert_int_equal(ret, SUCCESS);
	assert_ptr_equal(src.ptr, NULL);
	assert_int_equal(src.len, 0);
	assert_ptr_equal(dst.ptr, NULL);
	assert_int_equal(dst.len, 0);
}

/*
 * copy__malloc_ENOMEM - malloc() fail with ENOMEM
 */
static void
copy__malloc_ENOMEM(void **pdata_ptr)
{
	struct rpma_conn_private_data *src = *pdata_ptr;
	void *src_ptr = src->ptr;
	uint8_t src_len = src->len;

	/* configure mocks */
	will_return(__wrap__test_malloc, ENOMEM);

	/* run test */
	struct rpma_conn_private_data dst = {0};
	int ret = rpma_private_data_copy(&dst, src);

	/* verify the result */
	assert_int_equal(ret, RPMA_E_NOMEM);
	assert_ptr_equal(src->ptr, src_ptr);
	assert_int_equal(src->len, src_len);
	assert_ptr_equal(dst.ptr, NULL);
	assert_int_equal(dst.len, 0);
}

/*
 * copy__success - happy day scenario
 */
static void
copy__success(void **pdata_ptr)
{
	struct rpma_conn_private_data *src = *pdata_ptr;
	void *src_ptr = src->ptr;
	uint8_t src_len = src->len;

	/* configure mocks */
	will_return(__wrap__test_malloc, SUCCESS);

	/* run test */
	struct rpma_conn_private_data dst = {0};
	int ret = rpma_private_data_copy(&dst, src);

	/* verify the result */
	assert_int_equal(ret, SUCCESS);
	assert_ptr_equal(src->ptr, src_ptr);
	assert_int_equal(src->len, src_len);
	assert_int_equal(dst.len, src->len);
	assert_string_equal(dst.ptr, src->ptr);

	/* cleanup */
	rpma_private_data_discard(&dst);
	assert_int_equal(ret, SUCCESS);
	assert_ptr_equal(dst.ptr, NULL);
	assert_int_equal(dst.len, 0);
}

const struct CMUnitTest test_copy[] = {
	/* rpma_private_data_copy() unit tests */
	cmocka_unit_test(copy__ptr_NULL_len_0),
	cmocka_unit_test_setup_teardown(copy__malloc_ENOMEM,
		setup__private_data, teardown__private_data),
	cmocka_unit_test_setup_teardown(copy__success,
		setup__private_data, teardown__private_data),
	cmocka_unit_test(NULL)
};

int
main(int argc, char *argv[])
{
	return cmocka_run_group_tests(test_copy, NULL, NULL);
}
