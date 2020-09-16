// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020, Intel Corporation */

/*
 * client.c -- a client of the read-to-volatile example
 *
 * The client in this example reads data from the remote memory to a local
 * volatile one.
 */

#include <librpma.h>
#include <librpma_log.h>
#include <stdlib.h>
#include <stdio.h>

#include "common-conn.h"

#ifdef TEST_MOCK_MAIN
#include "cmocka_headers.h"
#include "cmocka_alloc.h"
#define main client_main
#endif

int
main(int argc, char *argv[])
{
	if (argc < 3) {
		fprintf(stderr, "usage: %s <server_address> <port>\n",
			argv[0]);
		exit(-1);
	}

	/* configure logging thresholds to see more details */
	rpma_log_set_threshold(RPMA_LOG_THRESHOLD, RPMA_LOG_LEVEL_INFO);
	rpma_log_set_threshold(RPMA_LOG_THRESHOLD_AUX, RPMA_LOG_LEVEL_INFO);

	/* parameters */
	char *addr = argv[1];
	char *port = argv[2];

	/* resources - general */
	struct rpma_peer *peer = NULL;
	struct rpma_conn *conn = NULL;
	struct rpma_completion cmpl;

	/*
	 * resources - memory regions:
	 * - src_* - a remote one which is a source for the read
	 * - dst_* - a local, volatile one which is a destination for the read
	 */
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
	if (ret)
		goto err_mr_free;

	/* establish a new connection to a server listening at addr:port */
	ret = client_connect(peer, addr, port, NULL, &conn);
	if (ret)
		goto err_mr_dereg;

	/* receive a memory info from the server */
	struct rpma_conn_private_data pdata;
	ret = rpma_conn_get_private_data(conn, &pdata);
	if (ret) {
		goto err_conn_disconnect;
	} else if (pdata.ptr == NULL) {
		fprintf(stderr,
				"The server has not provided a remote memory region. (the connection's private data is empty): %s",
				strerror(ret));
		goto err_conn_disconnect;
	}

	/*
	 * Create a remote memory registration structure from the received
	 * descriptor.
	 */
	struct common_data *dst_data = pdata.ptr;
	ret = rpma_mr_remote_from_descriptor(
			&dst_data->descriptors[dst_data->mr_desc_offset],
			dst_data->mr_desc_size,
			&src_mr);
	if (ret)
		goto err_conn_disconnect;

	/* get the remote memory region size */
	ret = rpma_mr_remote_get_size(src_mr, &src_size);
	if (ret) {
		goto err_mr_remote_delete;
	} else if (src_size > KILOBYTE) {
		fprintf(stderr,
				"Remote memory region size too big to reading to the sink buffer of the assumed size (%zu > %d)\n",
				src_size, KILOBYTE);
		goto err_mr_remote_delete;
	}

	/* post an RDMA read operation */
	ret = rpma_read(conn, dst_mr, 0, src_mr, 0, src_size,
			RPMA_F_COMPLETION_ALWAYS, NULL);
	if (ret)
		goto err_mr_remote_delete;

	/* wait for the completion to be ready */
	ret = rpma_conn_prepare_completions(conn);
	if (ret)
		goto err_mr_remote_delete;

	/* wait for a completion of the RDMA read */
	ret = rpma_conn_next_completion(conn, &cmpl);
	if (cmpl.op != RPMA_OP_READ) {
		fprintf(stderr,
				"rpma_conn_next_completion returned a completion of an unexpected operation: %d\n",
				cmpl.op);
	} else if (cmpl.op_status == IBV_WC_SUCCESS) {
		fprintf(stdout, "Read a message: %s\n", (char *)dst_ptr);
	}

err_mr_remote_delete:
	/* delete the remote memory region's structure */
	(void) rpma_mr_remote_delete(&src_mr);

err_conn_disconnect:
	(void) common_disconnect_and_wait_for_conn_close(&conn);

err_mr_dereg:
	/* deregister the memory region */
	(void) rpma_mr_dereg(&dst_mr);

err_mr_free:
	/* free the memory */
	free(dst_ptr);

err_peer_delete:
	/* delete the peer */
	(void) rpma_peer_delete(&peer);

	return ret;
}
