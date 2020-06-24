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
 * test_store__data_NULL -- data == NULL should prevent storing a private data
 */
static void
test_store__data_NULL(void **edata_ptr)
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
 * test_store__data_len_0 -- data_len == 0 should prevent storing a private data
 * object
 */
static void
test_store__data_len_0(void **edata_ptr)
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
 * test_store__data_NULL_data_len_0 -- data == NULL && data_len == 0 should
 * prevent storing a private data
 */
static void
test_store__data_NULL_data_len_0(void **edata_ptr)
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
 * test_store__malloc_ENOMEM -- malloc() fail with ENOMEM
 */
static void
test_store__malloc_ENOMEM(void **edata_ptr)
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

	rpma_private_data_discard(pdata);
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

/*
 * test_copy__ptr_NULL - ptr == NULL should prevent storing a private data
 */
static void
test_copy__ptr_NULL(void **unused)
{
	struct rpma_conn_private_data src = {NULL, 1};
	struct rpma_conn_private_data dst = {0};
	int ret = rpma_private_data_copy(&src, &dst);

	/* verify the result */
	assert_int_equal(ret, SUCCESS);
	assert_ptr_equal(src.ptr, NULL);
	assert_int_equal(src.len, 1);
	assert_ptr_equal(dst.ptr, NULL);
	assert_int_equal(dst.len, 0);
}

/*
 * test_copy__len_0 - len == 0 should prevent storing a private data
 */
static void
test_copy__len_0(void **unused)
{
	char buff = 0;
	struct rpma_conn_private_data src = {&buff, 0};
	struct rpma_conn_private_data dst = {0};
	int ret = rpma_private_data_copy(&src, &dst);

	/* verify the result */
	assert_int_equal(ret, SUCCESS);
	assert_ptr_equal(src.ptr, &buff);
	assert_int_equal(src.len, 0);
	assert_ptr_equal(dst.ptr, NULL);
	assert_int_equal(dst.len, 0);
}

/*
 * test_copy__ptr_NULL_len_0 - ptr == NULL and len == 0 should prevent storing a
 * private data
 */
static void
test_copy__ptr_NULL_len_0(void **unused)
{
	struct rpma_conn_private_data src = {NULL, 0};
	struct rpma_conn_private_data dst = {0};
	int ret = rpma_private_data_copy(&src, &dst);

	/* verify the result */
	assert_int_equal(ret, SUCCESS);
	assert_ptr_equal(src.ptr, NULL);
	assert_int_equal(src.len, 0);
	assert_ptr_equal(dst.ptr, NULL);
	assert_int_equal(dst.len, 0);
}

/*
 * test_copy__malloc_ENOMEM - malloc() fail with ENOMEM
 */
static void
test_copy__malloc_ENOMEM(void **pdata_ptr)
{
	struct rpma_conn_private_data *src = *pdata_ptr;
	void *src_ptr = src->ptr;
	uint8_t src_len = src->len;

	/* configure mocks */
	will_return(__wrap__test_malloc, ENOMEM);

	/* run test */
	struct rpma_conn_private_data dst = {0};
	int ret = rpma_private_data_copy(src, &dst);

	/* verify the result */
	assert_int_equal(ret, RPMA_E_NOMEM);
	assert_ptr_equal(src->ptr, src_ptr);
	assert_int_equal(src->len, src_len);
	assert_ptr_equal(dst.ptr, NULL);
	assert_int_equal(dst.len, 0);
}

/*
 * test_copy__success - happy day scenario
 */
static void
test_copy__success(void **pdata_ptr)
{
	struct rpma_conn_private_data *src = *pdata_ptr;
	void *src_ptr = src->ptr;
	uint8_t src_len = src->len;

	/* configure mocks */
	will_return(__wrap__test_malloc, SUCCESS);

	/* run test */
	struct rpma_conn_private_data dst = {0};
	int ret = rpma_private_data_copy(src, &dst);

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

int
main(int argc, char *argv[])
{
	const struct CMUnitTest tests[] = {
		/* rpma_private_data_store() unit tests */
		cmocka_unit_test_setup_teardown(test_store__data_NULL,
				setup_cm_event, NULL),
		cmocka_unit_test_setup_teardown(test_store__data_len_0,
				setup_cm_event, NULL),
		cmocka_unit_test_setup_teardown(
				test_store__data_NULL_data_len_0,
				setup_cm_event, NULL),
		cmocka_unit_test_setup_teardown(test_store__malloc_ENOMEM,
				setup_cm_event, NULL),

		/* rpma_private_data_store()/_discard() lifecycle */
		cmocka_unit_test_setup_teardown(test_lifecycle,
				setup_private_data, teardown_private_data),

		/* rpma_private_data_copy() unit tests */
		cmocka_unit_test(test_copy__ptr_NULL),
		cmocka_unit_test(test_copy__len_0),
		cmocka_unit_test(test_copy__ptr_NULL_len_0),
		cmocka_unit_test_setup_teardown(test_copy__malloc_ENOMEM,
				setup_private_data, teardown_private_data),
		cmocka_unit_test_setup_teardown(test_copy__success,
				setup_private_data, teardown_private_data),

	};

	return cmocka_run_group_tests(tests, NULL, NULL);
}
