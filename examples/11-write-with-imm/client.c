// SPDX-License-Identifier: BSD-3-Clause
/* Copyright (c) 2020-2021 Fujitsu */

/*
 * client.c -- a client of the write-with-imm example
 *
 * Please see README.md for a detailed description of this example.
 */

#include <librpma.h>
#include <limits.h>
#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>

#include "common-conn.h"

#define USAGE_STR "usage: %s <server_address> <port> <operation> <word> <imm>\n"

int
main(int argc, char *argv[])
{
	/* validate parameters */
	if (argc < 6) {
		fprintf(stderr, USAGE_STR, argv[0]);
		return -1;
	}

	/* configure logging thresholds to see more details */
	rpma_log_set_threshold(RPMA_LOG_THRESHOLD, RPMA_LOG_LEVEL_INFO);
	rpma_log_set_threshold(RPMA_LOG_THRESHOLD_AUX, RPMA_LOG_LEVEL_INFO);

	/* read common parameters */
	char *addr = argv[1];
	char *port = argv[2];
	char *operation = argv[3];
	char *word = argv[4];

	if (strcmp(operation, "write") && strcmp(operation, "write_atomic")) {
		fprintf(stderr,
			"'operation' argument only supports write or write_atomic\n");
		return -1;
	}

	uint64_t imm = strtoul(argv[5], NULL, 10);
	if (imm == ULONG_MAX && errno == ERANGE) {
		fprintf(stderr, "strtoul() overflowed\n");
		return -1;
	}

	if (imm > UINT32_MAX) {
		fprintf(stderr,
			"the provided immediate data is too big(%lu > %u)\n",
			imm, UINT32_MAX);
		return -1;
	}

	/* RPMA resources - general */
	struct rpma_peer *peer = NULL;
	struct rpma_mr_local *src_mr = NULL;
	struct rpma_mr_remote *dst_mr = NULL;
	struct rpma_conn *conn = NULL;
	struct rpma_completion cmpl;
	int ret;

	/* prepare memory */
	char *src = malloc_aligned(KILOBYTE);
	if (!src)
		return -1;

	/*
	 * lookup an ibv_context via the address and create a new peer using it
	 */
	ret = client_peer_via_address(addr, &peer);
	if (ret)
		goto err_mr_free;

	/* register the memory */
	ret = rpma_mr_reg(peer, src, KILOBYTE, RPMA_MR_USAGE_SEND, &src_mr);
	if (ret)
		goto err_peer_delete;

	/*
	 * establish a new connection to a server and send an immediate
	 * data for validation on the sever side
	 */
	struct rpma_conn_private_data src_pdata;
	src_pdata.ptr = (uint32_t *)&imm;
	src_pdata.len = sizeof(uint32_t);
	ret = client_connect(peer, addr, port, &src_pdata, &conn);
	if (ret)
		goto err_mr_dereg;

	/* obtain the remote memory description */
	struct rpma_conn_private_data dst_pdata;
	ret = rpma_conn_get_private_data(conn, &dst_pdata);
	if (ret)
		goto err_conn_disconnect;
	if (dst_pdata.len < sizeof(struct common_data)) {
		fprintf(stderr,
			"received connection's private data is too small (%u < %zu)\n",
		dst_pdata.len, sizeof(struct common_data));
		ret = -1;
		goto err_conn_disconnect;
	}

	struct common_data *dst_data = dst_pdata.ptr;
	ret = rpma_mr_remote_from_descriptor(&dst_data->descriptors[0],
			dst_data->mr_desc_size, &dst_mr);
	if (ret)
		goto err_conn_disconnect;

	/* send a message with immediate data to the server */
	strcpy(src, word);
	if (strcmp(operation, "write") == 0) {
		fprintf(stdout, "write a value %s with immediate data %u\n",
			src, (uint32_t)imm);
		ret = rpma_write_with_imm(conn, dst_mr, dst_data->data_offset,
				src_mr, 0, KILOBYTE, RPMA_F_COMPLETION_ALWAYS,
				(uint32_t)imm, NULL);
	} else {
		fprintf(stdout,
			"atomic write a value %s with immediate data %u\n",
			src, (uint32_t)imm);
		if (strlen(src) > RPMA_ATOMIC_WRITE_ALIGNMENT)
			fprintf(stdout,
				"atomic write %s will be truncated to %d bytes\n",
				src, RPMA_ATOMIC_WRITE_ALIGNMENT);
		ret = rpma_write_atomic_with_imm(conn, dst_mr,
				dst_data->data_offset, src_mr, 0,
				RPMA_F_COMPLETION_ALWAYS, (uint32_t)imm, NULL);
	}
	if (ret)
		goto err_conn_disconnect;

	/* prepare completions, get one and validate it */
	ret = rpma_conn_completion_wait(conn);
	if (ret)
		goto err_conn_disconnect;

	ret = rpma_conn_completion_get(conn, &cmpl);
	if (ret)
		goto err_conn_disconnect;

	if (cmpl.op_status != IBV_WC_SUCCESS) {
		fprintf(stderr,
			"an unexpected completion: %s\n",
			ibv_wc_status_str(cmpl.op_status));
		ret = -1;
	}

	if (cmpl.op != RPMA_OP_WRITE) {
		fprintf(stderr,
			"an unexpected type of operation (%d != %d)\n",
			cmpl.op, RPMA_OP_SEND);
		ret = -1;
	}

err_conn_disconnect:
	common_disconnect_and_wait_for_conn_close(&conn);

err_mr_dereg:
	/* deregister the memory regions */
	rpma_mr_dereg(&src_mr);

err_peer_delete:
	/* delete the peer object */
	rpma_peer_delete(&peer);

err_mr_free:
	/* free the memory */
	free(src);

	return ret;
}
