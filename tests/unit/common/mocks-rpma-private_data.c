/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2020, Intel Corporation
 */

/*
 * mocks-rpma-private_data.c -- librpma private_data.c module mocks
 */

#include <rdma/rdma_cma.h>
#include <string.h>
#include <librpma.h>

#include "cmocka_headers.h"

/*
 * rpma_private_data_store -- rpma_private_data_store() mock
 */
int
rpma_private_data_store(struct rdma_cm_event *edata,
		struct rpma_conn_private_data *pdata)
{
	const LargestIntegralType allowed_events[] = {
			RDMA_CM_EVENT_CONNECT_REQUEST,
			RDMA_CM_EVENT_ESTABLISHED};
	assert_non_null(edata);
	assert_in_set(edata->event, allowed_events,
		sizeof(allowed_events) / sizeof(allowed_events[0]));
	assert_non_null(pdata);
	assert_null(pdata->ptr);
	assert_int_equal(pdata->len, 0);

	int ret = mock_type(int);
	if (ret)
		return ret;

	pdata->ptr = (void *)edata->param.conn.private_data;
	pdata->len = edata->param.conn.private_data_len;

	return 0;
}

/*
 * rpma_private_data_discard -- rpma_private_data_discard() mock
 */
void
rpma_private_data_discard(struct rpma_conn_private_data *pdata)
{
	assert_non_null(pdata);
}
