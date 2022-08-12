// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020-2022, Intel Corporation */

/*
 * private_data.c -- a store for connections' private data
 */

#include <stdlib.h>
#include <string.h>

#include "private_data.h"
#include "debug.h"

#ifdef TEST_MOCK_ALLOC
#include "cmocka_alloc.h"
#endif

/*
 * rpma_private_data_store -- store a copy of the data provided via the CM event object
 */
int
rpma_private_data_store(struct rdma_cm_event *edata, struct rpma_conn_private_data *pdata)
{
	RPMA_DEBUG_TRACE;
	RPMA_FAULT_INJECTION(RPMA_E_NOMEM, {});

	const void *ptr = edata->param.conn.private_data;
	uint8_t len = edata->param.conn.private_data_len;

	if (ptr == NULL || len == 0)
		return 0;

	/* allocate a buffer for a copy of data from ptr */
	void *ptr_copy = malloc(len);
	if (ptr_copy == NULL)
		return RPMA_E_NOMEM;

	/* copy the data to the buffer */
	memcpy(ptr_copy, ptr, len);

	pdata->ptr = ptr_copy;
	pdata->len = len;

	return 0;
}

/*
 * rpma_private_data_delete -- free the private data
 */
void
rpma_private_data_delete(struct rpma_conn_private_data *pdata)
{
	RPMA_DEBUG_TRACE;

	free(pdata->ptr);

	pdata->ptr = NULL;
	pdata->len = 0;
}
