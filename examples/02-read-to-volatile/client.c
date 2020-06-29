// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020, Intel Corporation */

/*
 * client.c -- a client of the read example
 */

#include <librpma.h>
#include <stdlib.h>
#include <stdio.h>

#include "common.h"

#define KILOBYTE 1024

int
main(int argc, char *argv[])
{
	if (argc < 3) {
		fprintf(stderr, "usage: %s <server_address> <service>\n",
			argv[0]);
		exit(-1);
	}

	/* parameters */
	char *addr = argv[1];
	char *service = argv[2];

	/* resources - general */
	struct rpma_peer *peer = NULL;
	struct rpma_conn *conn = NULL;
	struct rpma_completion cmpl;

	/* resources - memory regions */
	void *dst_ptr = NULL;
	struct rpma_mr_local *dst_mr = NULL;
	struct rpma_mr_remote *src_mr = NULL;
	size_t src_size = 0;

	/*
	 * lookup an ibv_context via the address and create a new peer using it
	 */
	int ret = client_peer_via_address(addr, &peer);
	if (ret)
		return ret;

	/* allocate a memory */
	dst_ptr = malloc_aligned(KILOBYTE);
	if (dst_ptr == NULL) {
		ret = -1;
		goto err_peer_delete;
	}

	/* register the memory */
	ret = rpma_mr_reg(peer, dst_ptr, KILOBYTE, RPMA_MR_USAGE_READ_DST,
			RPMA_MR_PLT_VOLATILE, &dst_mr);
	if (ret) {
		print_error("rpma_mr_reg", ret);
		goto err_mr_free;
	}

	/* establish a new connection to a server listening at addr:service */
	ret = client_connect(peer, addr, service, &conn);
	if (ret)
		goto err_mr_dereg;

	/* receive a memory info from the server */
	struct rpma_conn_private_data pdata;
	ret = rpma_conn_get_private_data(conn, &pdata);
	if (ret) {
		print_error("rpma_conn_next_event", ret);
		goto err_conn_disconnect;
	} else if (pdata.ptr == NULL) {
		print_error("No data received.", ret);
		goto err_conn_disconnect;
	}

	/* deserialize the remote memory registration structure */
	ret = rpma_mr_deserialize(pdata.ptr, rpma_mr_serialize_get_size(),
			&src_mr);
	if (ret) {
		print_error("rpma_mr_deserialize", ret);
		goto err_conn_disconnect;
	}

	/* get the remote memory region size */
	ret = rpma_mr_remote_get_size(src_mr, &src_size);
	if (ret) {
		print_error("rpma_mr_remote_size", ret);
		goto err_mr_remote_delete;
	} else if (src_size > KILOBYTE) {
		fprintf(stderr, "Remote memory region size too big to reading "
				"to the sink buffer of the assumed size "
				"(%zu > %d)\n",
				src_size, KILOBYTE);
		goto err_mr_remote_delete;
	} else {
		fprintf(stdout, "Remote memory region size: %zu\n",
				src_size);
	}

	/* post an RDMA read operation */
	ret = rpma_read(conn, dst_mr, 0, src_mr, 0, src_size,
			RPMA_F_COMPLETION_ALWAYS, NULL);

	/* wait for a completion of the RDMA read */
	ret = rpma_conn_next_completion(conn, &cmpl);
	if (ret) {
		print_error("rpma_conn_next_completion", ret);
	} else if (cmpl.op != RPMA_OP_READ) {
		fprintf(stderr, "rpma_conn_next_completion returned a "
				"completion of an unexpected operation: %d\n",
				cmpl.op);
	} else if (cmpl.op_status != IBV_WC_SUCCESS) {
		fprintf(stderr, "rpma_conn_next_completion returned an "
				"unexpected operation status: %d\n",
				cmpl.op_status);
	} else {
		fprintf(stdout, "Read a message: %s\n", (char *)dst_ptr);
	}

err_mr_remote_delete:
	/* delete the remote memory region's structure */
	ret = rpma_mr_remote_delete(&src_mr);
	if (ret)
		print_error("rpma_mr_remote_delete", ret);

err_conn_disconnect:
	(void) client_disconnect(&conn);


err_mr_dereg:
	/* deregister the memory region */
	ret = rpma_mr_dereg(&dst_mr);
	if (ret)
		print_error("rpma_mr_dereg", ret);

err_mr_free:
	/* free the memory */
	free(dst_ptr);

err_peer_delete:
	/* delete the peer */
	ret = rpma_peer_delete(&peer);
	if (ret)
		print_error("rpma_peer_delete", ret);

	return ret;
}
