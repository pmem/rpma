// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020, Intel Corporation */

/*
 * common.h -- a common functions declarations for examples
 */

#ifndef EXAMPLES_COMMON
#define EXAMPLES_COMMON

#include <librpma.h>

#ifdef USE_LIBPMEM

/* signature marking the persistent contents as valid */
#define SIGNATURE_STR "RPMA_EXAMPLE_SIG"
#define SIGNATURE_LEN (strlen(SIGNATURE_STR) + 1)

#endif

struct common_data {
	rpma_mr_descriptor desc;
	size_t data_offset;
};

#define KILOBYTE 1024

void print_error_ex(const char *fname, int ret);

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
		const char *service, struct rpma_conn_private_data *pdata,
		struct rpma_conn **conn_ptr);

int server_listen(struct rpma_peer *peer, const char *addr, const char *service,
		struct rpma_ep **ep_ptr);

int server_accept_connection(struct rpma_ep *ep,
		struct rpma_conn_private_data *pdata,
		struct rpma_conn **conn_ptr);

int common_wait_for_conn_close_and_disconnect(struct rpma_conn **conn_ptr);
int common_disconnect_and_wait_for_conn_close(struct rpma_conn **conn_ptr);

#endif /* EXAMPLES_COMMON */
