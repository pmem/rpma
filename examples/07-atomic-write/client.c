// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020, Intel Corporation */

/*
 * client.c -- a client of the atomic-write example
 *
 * Please see README.md for a detailed description of this example.
 */

#include <librpma.h>
#include <librpma_log.h>
#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>

#define USAGE_STR "usage: %s <server_address> <port> [word1] [word2] [..]\n"

#include "common-conn.h"

#define FLUSH_ID	(void *)0xF01D /* a random identifier */
#define KILOBYTE	1024

int
main(int argc, char *argv[])
{
	/* validate parameters */
	if (argc < 3) {
		fprintf(stderr, USAGE_STR, argv[0]);
		exit(-1);
	}

	/* configure logging thresholds to see more details */
	rpma_log_set_threshold(RPMA_LOG_THRESHOLD, RPMA_LOG_LEVEL_INFO);
	rpma_log_set_threshold(RPMA_LOG_THRESHOLD_AUX, RPMA_LOG_LEVEL_INFO);

	/* read common parameters */
	char *addr = argv[1];
	char *port = argv[2];
	int ret;

	/* resources - memory region */
	void *mr_ptr = NULL;
	size_t mr_size = 0;
	// size_t data_offset = 0;
	enum rpma_mr_plt mr_plt = RPMA_MR_PLT_VOLATILE;
	struct rpma_mr_remote *dst_mr = NULL;
	size_t dst_size = 0;
	size_t dst_offset = 0;
	struct rpma_mr_local *src_mr = NULL;
	struct rpma_completion cmpl;

	/* if no pmem support or it is not provided */
	if (mr_ptr == NULL) {
		mr_ptr = malloc_aligned(KILOBYTE);
		if (mr_ptr == NULL)
			return -1;

		mr_size = KILOBYTE;
		mr_plt = RPMA_MR_PLT_VOLATILE;
	}

	/* RPMA resources */
	struct rpma_peer *peer = NULL;
	struct rpma_conn *conn = NULL;

	/*
	 * lookup an ibv_context via the address and create a new peer using it
	 */
	ret = client_peer_via_address(addr, &peer);
	if (ret)
		goto err_free;

	/* establish a new connection to a server listening at addr:port */
	ret = client_connect(peer, addr, port, NULL, &conn);
	if (ret)
		goto err_peer_delete;

	/* register the memory RDMA write */
	ret = rpma_mr_reg(peer, mr_ptr, mr_size,
			RPMA_MR_USAGE_WRITE_SRC | RPMA_MR_USAGE_READ_DST,
			mr_plt, &src_mr);
	if (ret)
		goto err_mr_dereg;

	/* obtain the remote memory description */
	struct rpma_conn_private_data pdata;
	ret = rpma_conn_get_private_data(conn, &pdata);
	if (ret != 0 || pdata.len < sizeof(struct common_data))
		goto err_conn_disconnect;

	/*
	 * Create a remote memory registration structure from the received
	 * descriptor.
	 */
	struct common_data *dst_data = pdata.ptr;
	dst_offset = dst_data->data_offset;
	ret = rpma_mr_remote_from_descriptor(&dst_data->desc, &dst_mr);
	if (ret)
		goto err_conn_disconnect;

	/* get the remote memory region size */
	ret = rpma_mr_remote_get_size(dst_mr, &dst_size);
	if (ret)
		goto err_mr_remote_delete;

	ret = rpma_read(conn, src_mr, 0, dst_mr, dst_offset,
			8, RPMA_F_COMPLETION_ALWAYS, NULL);
	if (ret)
		goto err_conn_disconnect;

	uint64_t log_data_offset = *(uint64_t *)mr_ptr;

	printf("used value: %lu\n", log_data_offset);

	if (dst_size < log_data_offset) {
		fprintf(stderr,
				"Remote memory region size too small for writing the data of the assumed size (%zu < %ld)\n",
				dst_size, log_data_offset);
		goto err_mr_remote_delete;
	}

	/* wait for the completion to be ready */
	ret = rpma_conn_prepare_completions(conn);
	if (ret)
		goto err_conn_disconnect;

	if (argc == 3)
		goto err_conn_disconnect;

	for (int i = 3; i < argc; ++i) {
		char *word = mr_ptr;
		strcpy(word, argv[i]);
		size_t word_size = strlen(word) + 1;

		ret = rpma_write(conn, dst_mr, log_data_offset, src_mr, 0,
				word_size, RPMA_F_COMPLETION_ON_ERROR, NULL);
		if (ret)
			goto err_conn_disconnect;

		ret = rpma_flush(conn, dst_mr, dst_offset, 8,
				RPMA_FLUSH_TYPE_PERSISTENT,
				RPMA_F_COMPLETION_ALWAYS, FLUSH_ID);
		if (ret)
			goto err_conn_disconnect;

		log_data_offset += word_size;
		*(uint64_t *)mr_ptr = log_data_offset;

		ret = rpma_write_atomic(conn, dst_mr, dst_offset, src_mr, 0,
				RPMA_F_COMPLETION_ON_ERROR, NULL);
		if (ret)
			goto err_conn_disconnect;

		ret = rpma_flush(conn, dst_mr, dst_offset, 8,
				RPMA_FLUSH_TYPE_PERSISTENT,
				RPMA_F_COMPLETION_ALWAYS, FLUSH_ID);
		if (ret)
			goto err_conn_disconnect;

		ret = rpma_conn_next_completion(conn, &cmpl);
		if (ret)
			goto err_conn_disconnect;

		if (cmpl.op_status != IBV_WC_SUCCESS) {
			(void) fprintf(stderr, "rpma_read failed with %d\n",
					cmpl.op_status);
			goto err_conn_disconnect;
		}
	}

err_conn_disconnect:
	(void) common_disconnect_and_wait_for_conn_close(&conn);

err_mr_remote_delete:
	/* delete the remote memory region's structure */
	(void) rpma_mr_remote_delete(&dst_mr);

err_mr_dereg:
	/* deregister the memory region */
	(void) rpma_mr_dereg(&src_mr);

err_peer_delete:
	/* delete the peer */
	(void) rpma_peer_delete(&peer);

err_free:
	if (mr_ptr != NULL)
		free(mr_ptr);

	return ret;
}
