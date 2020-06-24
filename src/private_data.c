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
 * rpma_private_data_store -- store a copy of the data provided via the CM event
 * object
 */
int
rpma_private_data_store(struct rdma_cm_event *edata,
		struct rpma_conn_private_data *pdata)
{
	ASSERTne(edata, NULL);
	ASSERTne(pdata, NULL);
	ASSERT(edata->event == RDMA_CM_EVENT_CONNECT_REQUEST ||
			edata->event == RDMA_CM_EVENT_ESTABLISHED);

	const void *ptr = edata->param.conn.private_data;
	uint8_t len = edata->param.conn.private_data_len;

	if (ptr == NULL || len == 0) {
		pdata->ptr = NULL;
		pdata->len = 0;
		return 0;
	}

	/* allocate a buffer for a copy of data from ptr */
	void *ptr_copy = Malloc(len);
	if (ptr_copy == NULL) {
		ASSERTeq(errno, ENOMEM);
		return RPMA_E_NOMEM;
	}

	/* copy the data to the buffer */
	memcpy(ptr_copy, ptr, len);

	pdata->ptr = ptr_copy;
	pdata->len = len;

	return 0;
}

/*
 * rpma_private_data_copy -- copy private data from src to dst
 */
int
rpma_private_data_copy(struct rpma_conn_private_data *dst,
		struct rpma_conn_private_data *src)
{
	if (src->ptr == NULL)
		return 0;

	dst->ptr = Malloc(src->len);
	if (dst->ptr == NULL)
		return RPMA_E_NOMEM;

	memcpy(dst->ptr, src->ptr, src->len);
	dst->len = src->len;

	return 0;
}

/*
 * rpma_private_data_discard -- free the private data
 */
void
rpma_private_data_discard(struct rpma_conn_private_data *pdata)
{
	ASSERTne(pdata, NULL);

	Free(pdata->ptr);

	pdata->ptr = NULL;
	pdata->len = 0;
}
