// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020-2022, Intel Corporation */
/* Copyright 2021, Fujitsu */

/*
 * private_data-common.c -- the private_data unit tests common functions
 */

#include <rdma/rdma_cma.h>
#include <stdlib.h>

#include "cmocka_headers.h"
#include "private_data.h"
#include "private_data-common.h"
#include "librpma.h"

/*
 * setup__cm_event -- setup test environment with a valid cm_event
 */
int
setup__cm_event(void **edata_ptr)
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
 * setup__private_data -- prepare a valid private data object
 */
int
setup__private_data(void **pdata_ptr)
{
	struct rdma_cm_event edata = {0};
	char buff[] = DEFAULT_VALUE;

	edata.event = CM_EVENT_VALID;
	edata.param.conn.private_data = buff;
	edata.param.conn.private_data_len = DEFAULT_LEN;

	/* configure mocks */
	will_return(__wrap__test_malloc, MOCK_OK);

	static struct rpma_conn_private_data pdata = {0};
	int ret = rpma_private_data_store(&edata, &pdata);
	assert_int_equal(ret, MOCK_OK);
	assert_non_null(pdata.ptr);
	assert_string_equal(pdata.ptr, DEFAULT_VALUE);
	assert_int_equal(pdata.len, DEFAULT_LEN);

	*pdata_ptr = &pdata;

	return 0;
}

/*
 * teardown__private_data -- delete the private data object
 */
int
teardown__private_data(void **pdata_ptr)
{
	struct rpma_conn_private_data *pdata = *pdata_ptr;

	rpma_private_data_delete(pdata);
	assert_null(pdata->ptr);
	assert_int_equal(pdata->len, 0);

	return 0;
}
