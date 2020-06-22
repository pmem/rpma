/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2020, Intel Corporation
 */

/*
 * private_data.c -- a store for connections' private data
 */

#include "cmocka_alloc.h"
#include "private_data.h"
#include "out.h"

/*
 * rpma_private_data_new -- create a new private data object for storing a copy
 * of the data provided via the CM event object.
 */
int
rpma_private_data_new(struct rdma_cm_event *edata,
		struct rpma_conn_private_data **pdata_ptr)
{
	if (edata == NULL || pdata_ptr == NULL)
		return RPMA_E_INVAL;

	if (edata->event != RDMA_CM_EVENT_CONNECT_REQUEST &&
			edata->event != RDMA_CM_EVENT_ESTABLISHED)
		return RPMA_E_INVAL;

	const void *ptr = edata->param.conn.private_data;
	uint8_t len = edata->param.conn.private_data_len;

	if (ptr == NULL || len == 0)
		return 0;

	/* allocate a buffer for a copy of data from ptr */
	void *ptr_copy = Malloc(len);
	if (ptr_copy == NULL) {
		ASSERTeq(errno, ENOMEM);
		return RPMA_E_NOMEM;
	}

	/* copy the data to the buffer */
	memcpy(ptr_copy, ptr, len);

	struct rpma_conn_private_data *pdata = Malloc(sizeof(*pdata));
	if (pdata == NULL) {
		ASSERTeq(errno, ENOMEM);
		Free(ptr_copy);
		return RPMA_E_NOMEM;
	}

	pdata->ptr = ptr_copy;
	pdata->len = len;
	*pdata_ptr = pdata;

	return 0;
}

/*
 * rpma_private_data_delete -- free the private data object.
 */
int
rpma_private_data_delete(struct rpma_conn_private_data **pdata_ptr)
{
	if (pdata_ptr == NULL)
		return RPMA_E_INVAL;

	struct rpma_conn_private_data *pdata = *pdata_ptr;
	if (pdata == NULL)
		return 0;

	Free(pdata->ptr);
	Free(pdata);
	*pdata_ptr = NULL;

	return 0;
}
