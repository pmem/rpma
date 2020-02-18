/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2020, Intel Corporation
 */

/*
 * msg.h -- definitions of librpma msg entry points (EXPERIMENTAL)
 *
 * This library provides low-level support for remote access to persistent
 * memory utilizing RDMA-capable RNICs.
 *
 * See librpma(7) for details.
 */

#ifndef LIBRPMA_MSG_H
#define LIBRPMA_MSG_H 1

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <base.h>

int rpma_msg_get_ptr(struct rpma_connection *conn, void **ptr);

int rpma_connection_send(struct rpma_connection *conn, void *ptr);

#ifdef __cplusplus
}
#endif
#endif /* librpma.h */
