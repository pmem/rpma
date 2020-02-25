/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2019-2020, Intel Corporation
 */

/*
 * librpma/msg.h -- definitions of librpma msg entry points (EXPERIMENTAL)
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

#include <librpma/base.h>

int rpma_send_buffer_get(struct rpma_conn *conn, size_t buff_size,
		void **buff);

int rpma_send(struct rpma_conn *conn, void *buff);

int rpma_recv_buffer_get(struct rpma_conn *conn, size_t *buff_size,
		void **buff);

int rpma_recv_buffer_return(struct rpma_conn *conn, void **buff);

#ifdef __cplusplus
}
#endif
#endif /* LIBRPMA_MSG_H */
