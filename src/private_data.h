/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2020, Intel Corporation
 */

/*
 * private_data.h -- a store for connections' private data (definitions)
 */

#ifndef LIBRPMA_PRIVATE_DATA_H
#define LIBRPMA_PRIVATE_DATA_H

#include <rdma/rdma_cma.h>

#include "librpma.h"

/*
 * ERRORS
 * rpma_private_data_store() can fail with the following error:
 *
 * - RPMA_E_NOMEM - out of memory
 */
int rpma_private_data_store(struct rdma_cm_event *edata,
		struct rpma_conn_private_data *pdata);

/*
 * ASSUMPTIONS
 * - src != NULL and dst != NULL
 * - dst == {NULL, 0}
 * - src == {NULL, 0} or (src->ptr != NULL && src->len != 0)
 *
 * ERRORS
 * rpma_private_data_copy() can fail with the following error:
 *
 * - RPMA_E_NOMEM - out of memory
 */
int rpma_private_data_copy(struct rpma_conn_private_data *src,
		struct rpma_conn_private_data *dst);

/*
 * The function cannot fail.
 */
void rpma_private_data_discard(struct rpma_conn_private_data *pdata);

#endif /* LIBRPMA_PRIVATE_DATA_H */
