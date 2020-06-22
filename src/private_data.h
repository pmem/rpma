/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2020, Intel Corporation
 */

/*
 * private_data.h -- a store for connections private data (definitions)
 */

#ifndef LIBRPMA_PRIVATE_DATA_H
#define LIBRPMA_PRIVATE_DATA_H

#include <rdma/rdma_cma.h>

#include "librpma.h"

/*
 * ERRORS
 * rpma_pdata_new() can fail with the following error:
 *
 * - RPMA_E_INVAL - edata or pdata_ptr is NULL or
 *   (edata->event != RDMA_CM_EVENT_CONNECT_REQUEST and
 *    edata->event != RDMA_CM_EVENT_ESTABLISHED)
 * - RPMA_E_NOMEM - out of memory
 */
int rpma_private_data_new(struct rdma_cm_event *edata,
		struct rpma_conn_private_data **pdata_ptr);

/*
 * ERRORS
 * rpma_pdata_delete() can fail with the following errors:
 *
 * - RPMA_E_INVAL - pdata_ptr is NULL
 */
int rpma_private_data_delete(struct rpma_conn_private_data **pdata_ptr);

#endif /* LIBRPMA_PRIVATE_DATA_H */
