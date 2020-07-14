// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020, Intel Corporation */

/*
 * server.c -- a server of the read-to-volatile example
 *
 * The server in this example, if provided (and capable of), prepares a local
 * persistent memory and registers it as a reading destination. After
 * the connection is established the server receives the client's memory region
 * registered as read source. The servers performs read from the remote memory
 * region to a local memory region.
 *
 * If the server does not have a pmem path (or it is not capable to use pmem
 * at all) it uses DRAM instead.
 */

#include <inttypes.h>
#include <librpma.h>
#include <stdlib.h>
#include <stdio.h>

#ifdef USE_LIBPMEM
#include <libpmem.h>

#define USAGE_PMEM " [<pmem-path>]"
#else
#define USAGE_PMEM
#endif

#include "common.h"

int
main(int argc, char *argv[])
{
	/* validate parameters */
	if (argc < 3) {
		fprintf(stderr,
		    "usage: %s <server_address> <service>" USAGE_PMEM "\n",
		    argv[0]);
		exit(-1);
	}

	/* read common parameters */
	char *addr = argv[1];
	char *service = argv[2];
	int ret;

	/* resources - memory region */
	void *dst_ptr = NULL;
	size_t dst_size = 0;
	size_t dst_offset = 0;
	enum rpma_mr_plt dst_plt = RPMA_MR_PLT_VOLATILE;
	int dst_initialized = 0;
	struct rpma_mr_local *dst_mr = NULL;
	struct rpma_mr_remote *src_mr = NULL;

#ifdef USE_LIBPMEM
	if (argc >= 4) {
		/* map pmem */
		char *path = argv[3];
		ret = common_pmem_map(path, &dst_ptr, &dst_size, &dst_offset,
				&dst_initialized);
		if (ret)
			return -1;

		/* validate pmem is big enough */
		if (dst_size - dst_offset < KILOBYTE) {
			fprintf(stderr, "%s too small (%zu < %zu)\n",
					path, dst_size, KILOBYTE + dst_offset);
			(void) pmem_unmap(dst_ptr, dst_size);
			return -1;
		}
		dst_plt = RPMA_MR_PLT_PERSISTENT;
	}
#endif

	/* if no pmem support or it is not provided */
	if (dst_ptr == NULL) {
		dst_ptr = malloc_aligned(KILOBYTE);
		if (dst_ptr == NULL)
			return -1;

		dst_size = KILOBYTE;
	}

	/* RPMA resources */
	struct rpma_peer *peer = NULL;
	struct rpma_ep *ep = NULL;
	struct rpma_conn *conn = NULL;
	struct rpma_completion cmpl;

	/*
	 * lookup an ibv_context via the address and create a new peer using it
	 */
	ret = server_peer_via_address(addr, &peer);
	if (ret)
		goto err_free;

	/* start a listening endpoint at addr:service */
	ret = server_listen(peer, addr, service, &ep);
	if (ret)
		goto err_peer_delete;

	/* register the memory */
	ret = rpma_mr_reg(peer, dst_ptr, dst_size, RPMA_MR_USAGE_READ_DST,
			dst_plt, &dst_mr);
	if (ret) {
		print_error_ex("rpma_mr_reg", ret);
		goto err_ep_shutdown;
	}

	/*
	 * Wait for an incoming connection request, accept it and wait for its
	 * establishment.
	 */
	ret = server_accept_connection(ep, NULL, &conn);
	if (ret)
		goto err_mr_dereg;

	/* obtain the remote memory description */
	struct rpma_conn_private_data pdata;
	ret = rpma_conn_get_private_data(conn, &pdata);
	if (ret != 0 || pdata.len < sizeof(struct common_data))
		goto err_disconnect;

	struct common_data *src_data = pdata.ptr;
	ret = rpma_mr_remote_from_descriptor(&src_data->desc, &src_mr);
	if (ret)
		goto err_disconnect;

	if (dst_initialized) {
		(void) printf("Old value: %s\n", (char *)dst_ptr + dst_offset);
	}

	ret = rpma_read(conn, dst_mr, dst_offset, src_mr, src_data->data_offset,
			src_data->data_len, RPMA_F_COMPLETION_ALWAYS, NULL);
	if (ret)
		goto err_disconnect;

	ret = rpma_conn_next_completion(conn, &cmpl);
	if (ret)
		goto err_disconnect;

#ifdef USE_LIBPMEM
	if (dst_plt == RPMA_MR_PLT_PERSISTENT) {
		pmem_persist((char *)dst_ptr + dst_offset, src_data->data_len);

		if (!dst_initialized)
			common_write_signature(dst_ptr);
	}
#endif

	(void) printf("New value: %s\n", (char *)dst_ptr + dst_offset);

err_disconnect:
	/*
	 * Wait for RPMA_CONN_CLOSED, disconnect and delete the connection
	 * structure.
	 */
	ret = common_disconnect_and_wait_for_conn_close(&conn);

err_mr_dereg:
	/* deregister the memory region */
	ret = rpma_mr_dereg(&dst_mr);
	if (ret)
		print_error_ex("rpma_mr_dereg", ret);

err_ep_shutdown:
	/* shutdown the endpoint */
	ret = rpma_ep_shutdown(&ep);
	if (ret)
		print_error_ex("rpma_ep_shutdown", ret);

err_peer_delete:
	/* delete the peer object */
	ret = rpma_peer_delete(&peer);
	if (ret)
		print_error_ex("rpma_peer_delete", ret);

err_free:
#ifdef USE_LIBPMEM
	if (dst_plt == RPMA_MR_PLT_PERSISTENT) {
		pmem_unmap(dst_ptr, dst_size);
		dst_ptr = NULL;
	}
#endif

	if (dst_ptr != NULL)
		free(dst_ptr);

	return ret;
}
