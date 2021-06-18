// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020, Intel Corporation */
/* Copyright 2021, Fujitsu */

/*
 * private_data-store.c -- the rpma_private_data_store() unit tests
 *
 * API covered:
 * -rpma_private_data_store()
 */

#include <rdma/rdma_cma.h>
#include <stdlib.h>

#include "cmocka_headers.h"
#include "private_data.h"
#include "private_data-common.h"
#include "librpma.h"

/*
 * store__data_NULL -- data == NULL should prevent storing a private data
 */
static void
store__data_NULL(void **edata_ptr)
{
	struct rdma_cm_event *edata = *edata_ptr;
	edata->param.conn.private_data = NULL;

	/* run test */
	struct rpma_conn_private_data pdata = {0};
	int ret = rpma_private_data_store(*edata_ptr, &pdata);

	/* verify the result */
	assert_int_equal(ret, MOCK_OK);
	assert_ptr_equal(pdata.ptr, NULL);
	assert_int_equal(pdata.len, 0);
}

/*
 * store__data_len_0 -- data_len == 0 should prevent storing a private data
 * object
 */
static void
store__data_len_0(void **edata_ptr)
{
	struct rdma_cm_event *edata = *edata_ptr;
	edata->param.conn.private_data_len = 0;

	/* run test */
	struct rpma_conn_private_data pdata = {0};
	int ret = rpma_private_data_store(*edata_ptr, &pdata);

	/* verify the result */
	assert_int_equal(ret, MOCK_OK);
	assert_ptr_equal(pdata.ptr, NULL);
	assert_int_equal(pdata.len, 0);
}

/*
 * store__data_NULL_data_len_0 -- data == NULL && data_len == 0 should
 * prevent storing a private data
 */
static void
store__data_NULL_data_len_0(void **edata_ptr)
{
	struct rdma_cm_event *edata = *edata_ptr;
	edata->param.conn.private_data = NULL;
	edata->param.conn.private_data_len = 0;

	/* run test */
	struct rpma_conn_private_data pdata = {0};
	int ret = rpma_private_data_store(*edata_ptr, &pdata);

	/* verify the result */
	assert_int_equal(ret, MOCK_OK);
	assert_ptr_equal(pdata.ptr, NULL);
	assert_int_equal(pdata.len, 0);
}

/*
 * store__malloc_ERRNO -- malloc() fails with MOCK_ERRNO
 */
static void
store__malloc_ERRNO(void **edata_ptr)
{
	/* configure mocks */
	will_return(__wrap__test_malloc, MOCK_ERRNO);

	/* run test */
	struct rpma_conn_private_data pdata = {0};
	int ret = rpma_private_data_store(*edata_ptr, &pdata);

	/* verify the result */
	assert_int_equal(ret, RPMA_E_NOMEM);
	assert_ptr_equal(pdata.ptr, NULL);
		assert_int_equal(pdata.len, 0);
}

/*
 * test_lifecycle - happy day scenario
 */
static void
test_lifecycle(void **unused)
{
	/*
	 * the thing is done by setup__private_data() and
	 * teardown__private_data()
	 */
}

static const struct CMUnitTest test_store[] = {
	/* rpma_private_data_store() unit tests */
	cmocka_unit_test_setup_teardown(store__data_NULL,
		setup__cm_event, NULL),
	cmocka_unit_test_setup_teardown(store__data_len_0,
		setup__cm_event, NULL),
	cmocka_unit_test_setup_teardown(store__data_NULL_data_len_0,
		setup__cm_event, NULL),
	cmocka_unit_test_setup_teardown(store__malloc_ERRNO,
		setup__cm_event, NULL),

	/* rpma_private_data_store()/_discard() lifecycle */
	cmocka_unit_test_setup_teardown(test_lifecycle,
		setup__private_data, teardown__private_data),
	cmocka_unit_test(NULL)
};

int
main(int argc, char *argv[])
{
	return cmocka_run_group_tests(test_store, NULL, NULL);
}
