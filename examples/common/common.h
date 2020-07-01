// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020, Intel Corporation */

/*
 * common.h -- a common functions declarations for examples
 */

#ifndef EXAMPLES_COMMON
#define EXAMPLES_COMMON

void print_error(const char *fname, int ret);

void *malloc_aligned(size_t size);

int common_peer_via_address(const char *addr,
		enum rpma_util_ibv_context_type type,
		struct rpma_peer **peer_ptr);

#define client_peer_via_address(addr, peer_ptr) \
		common_peer_via_address(addr, RPMA_UTIL_IBV_CONTEXT_REMOTE, \
				peer_ptr)

#define server_peer_via_address(addr, peer_ptr) \
		common_peer_via_address(addr, RPMA_UTIL_IBV_CONTEXT_LOCAL, \
				peer_ptr)

int client_connect(struct rpma_peer *peer, const char *addr,
		const char *service, struct rpma_conn **conn_ptr);

int client_disconnect(struct rpma_conn **conn_ptr);

int server_listen(struct rpma_peer *peer, const char *addr, const char *service,
		struct rpma_ep **ep_ptr);

int server_accept_connection(struct rpma_ep *ep,
		struct rpma_conn_private_data *pdata,
		struct rpma_conn **conn_ptr);

int server_disconnect(struct rpma_conn **conn_ptr);

#endif /* EXAMPLES_COMMON */
