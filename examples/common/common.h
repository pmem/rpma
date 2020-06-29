// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020, Intel Corporation */

/*
 * common.h -- a common functions declarations for examples
 */

#ifndef EXAMPLES_COMMON
#define EXAMPLES_COMMON

void print_error(const char *fname, int ret);

void *malloc_aligned(size_t size);

int client_peer_via_address(const char *addr, struct rpma_peer **peer_ptr);

int client_connect(struct rpma_peer *peer, const char *addr,
		const char *service, struct rpma_conn **conn_ptr);

int client_disconnect(struct rpma_conn **conn_ptr);

int server_peer_via_address(const char *addr, struct rpma_peer **peer_ptr);

int server_listen(struct rpma_peer *peer, const char *addr, const char *service,
		struct rpma_ep **ep_ptr);

int server_connect(struct rpma_ep *ep, struct rpma_conn_private_data *pdata,
		struct rpma_conn **conn_ptr);

int server_disconnect(struct rpma_conn **conn_ptr);

#endif /* EXAMPLES_COMMON */
