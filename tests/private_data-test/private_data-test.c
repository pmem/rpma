/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2020, Intel Corporation
 */

/*
 * private_data-test.c -- unit tests of the private_data module
 */

#include <rdma/rdma_cma.h>
#include <stdlib.h>

#include "cmocka_headers.h"
#include "private_data.h"
#include "librpma.h"

/*
 * Both RDMA_CM_EVENT_CONNECT_REQUEST and RDMA_CM_EVENT_ESTABLISHED are valid.
 * After the event type check, the behavior should not differ.
 */
#define CM_EVENT_VALID		RDMA_CM_EVENT_CONNECT_REQUEST

#define DEFAULT_VALUE		"The default one"
#define DEFAULT_LEN			(strlen(DEFAULT_VALUE) + 1)

#define SUCCESS		0

void *__real__test_malloc(size_t size);

/*
 * __wrap__test_malloc -- malloc() mock
 */
void *
__wrap__test_malloc(size_t size)
{
	int err = mock_type(int);

	if (err) {
		errno = err;
		return NULL;
	}

	return __real__test_malloc(size);
}

/*
 * setup_cm_event -- setup test environment with a valid cm_event
 */
static int
setup_cm_event(void **edata_ptr)
{
	static struct rdma_cm_event edata = {0};
	static char buff[] = DEFAULT_VALUE;

	edata.event = CM_EVENT_VALID;
	edata.param.conn.private_data = buff;
	edata.param.conn.private_data_len = DEFAULT_LEN;

	*edata_ptr = &edata;

	return 0;
}

/*
 * test_new__data_NULL -- data == NULL should prevent storing a private data
 */
static void
test_new__data_NULL(void **edata_ptr)
{
	struct rdma_cm_event *edata = *edata_ptr;
	edata->param.conn.private_data = NULL;

	/* run test */
	struct rpma_conn_private_data pdata = {0};
	int ret = rpma_private_data_store(*edata_ptr, &pdata);

	/* verify the result */
	assert_int_equal(ret, SUCCESS);
	assert_ptr_equal(pdata.ptr, NULL);
	assert_int_equal(pdata.len, 0);
}

/*
 * test_new__data_len_0 -- data_len == 0 should prevent storing a private data
 * object
 */
static void
test_new__data_len_0(void **edata_ptr)
{
	struct rdma_cm_event *edata = *edata_ptr;
	edata->param.conn.private_data_len = 0;

	/* run test */
	struct rpma_conn_private_data pdata = {0};
	int ret = rpma_private_data_store(*edata_ptr, &pdata);

	/* verify the result */
	assert_int_equal(ret, SUCCESS);
	assert_ptr_equal(pdata.ptr, NULL);
	assert_int_equal(pdata.len, 0);
}

/*
 * test_new__data_NULL_data_len_0 -- data == NULL && data_len == 0 should
 * prevent storing a private data
 */
static void
test_new__data_NULL_data_len_0(void **edata_ptr)
{
	struct rdma_cm_event *edata = *edata_ptr;
	edata->param.conn.private_data = NULL;
	edata->param.conn.private_data_len = 0;

	/* run test */
	struct rpma_conn_private_data pdata = {0};
	int ret = rpma_private_data_store(*edata_ptr, &pdata);

	/* verify the result */
	assert_int_equal(ret, SUCCESS);
	assert_ptr_equal(pdata.ptr, NULL);
	assert_int_equal(pdata.len, 0);
}

/*
 * test_new__malloc_ENOMEM -- malloc() fail with ENOMEM
 */
static void
test_new__1st_malloc_ENOMEM(void **edata_ptr)
{
	/* configure mocks */
	will_return(__wrap__test_malloc, ENOMEM);

	/* run test */
	struct rpma_conn_private_data pdata = {0};
	int ret = rpma_private_data_store(*edata_ptr, &pdata);

	/* verify the result */
	assert_int_equal(ret, RPMA_E_NOMEM);
	assert_ptr_equal(pdata.ptr, NULL);
		assert_int_equal(pdata.len, 0);
}

/*
 * setup_private_data -- prepare a valid private data object
 */
static int
setup_private_data(void **pdata_ptr)
{
	struct rdma_cm_event edata = {0};
	char buff[] = DEFAULT_VALUE;

	edata.event = CM_EVENT_VALID;
	edata.param.conn.private_data = buff;
	edata.param.conn.private_data_len = DEFAULT_LEN;

	/* configure mocks */
	will_return(__wrap__test_malloc, SUCCESS);

	static struct rpma_conn_private_data pdata = {0};
	int ret = rpma_private_data_store(&edata, &pdata);
	assert_int_equal(ret, SUCCESS);
	assert_non_null(pdata.ptr);
	assert_string_equal(pdata.ptr, DEFAULT_VALUE);
	assert_int_equal(pdata.len, DEFAULT_LEN);

	*pdata_ptr = &pdata;

	return 0;
}

/*
 * teardown_private_data -- delete the private data object
 */
static int
teardown_private_data(void **pdata_ptr)
{
	struct rpma_conn_private_data *pdata = *pdata_ptr;

	int ret = rpma_private_data_discard(pdata);
	assert_int_equal(ret, SUCCESS);
	assert_null(pdata->ptr);
	assert_int_equal(pdata->len, 0);

	return 0;
}

/*
 * test_lifecycle - happy day scenario
 */
static void
test_lifecycle(void **unused)
{
	/*
	 * the thing is done by setup_private_data() and
	 * teardown_private_data()
	 */
}

int
main(int argc, char *argv[])
{
	const struct CMUnitTest tests[] = {
		/* rpma_private_data_new() unit tests */
		cmocka_unit_test_setup_teardown(test_new__data_NULL,
				setup_cm_event, NULL),
		cmocka_unit_test_setup_teardown(test_new__data_len_0,
				setup_cm_event, NULL),
		cmocka_unit_test_setup_teardown(test_new__data_NULL_data_len_0,
				setup_cm_event, NULL),
		cmocka_unit_test_setup_teardown(test_new__1st_malloc_ENOMEM,
				setup_cm_event, NULL),

		/* rpma_private_data_new()/_delete() lifecycle */
		cmocka_unit_test_setup_teardown(test_lifecycle,
				setup_private_data, teardown_private_data),
	};

	return cmocka_run_group_tests(tests, NULL, NULL);
}
