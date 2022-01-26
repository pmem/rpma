// SPDX-License-Identifier: BSD-3-Clause
/* Copyright (c) 2021 Fujitsu */
/* Copyright 2021-2022, Intel Corporation */

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

#define USAGE_STR "usage: %s <server_address> <port> <imm>\n"

int
main(int argc, char *argv[])
{
	/* validate parameters */
	if (argc < 4) {
		fprintf(stderr, USAGE_STR, argv[0]);
		return -1;
	}

	/* configure logging thresholds to see more details */
	rpma_log_set_threshold(RPMA_LOG_THRESHOLD, RPMA_LOG_LEVEL_INFO);
	rpma_log_set_threshold(RPMA_LOG_THRESHOLD_AUX, RPMA_LOG_LEVEL_INFO);

	/* read common parameters */
	char *addr = argv[1];
	char *port = argv[2];

	uint64_t imm = strtoul(argv[3], NULL, 10);
	if (imm == ULONG_MAX && errno == ERANGE) {
		fprintf(stderr, "strtoul() overflow\n");
		return -1;
	}

	if (imm > UINT32_MAX) {
		fprintf(stderr,
			"The provided immediate data is too big (%lu > %u)\n",
			imm, UINT32_MAX);
		return -1;
	}

	/* RPMA resources - general */
	struct rpma_peer *peer = NULL;
	struct rpma_mr_local *src_mr = NULL;
	struct rpma_mr_remote *dst_mr = NULL;
	struct rpma_conn *conn = NULL;
	struct ibv_wc wc;
	int ret;

	/* prepare memory */
	uint32_t *src = malloc_aligned(KILOBYTE);
	if (!src)
		return -1;

	/*
	 * lookup an ibv_context via the address and create a new peer using it
	 */
	ret = client_peer_via_address(addr, &peer);
	if (ret)
		goto err_mr_free;

	/* register the memory */
	ret = rpma_mr_reg(peer, src, KILOBYTE, RPMA_MR_USAGE_WRITE_SRC,
			&src_mr);
	if (ret)
		goto err_peer_delete;

	/* establish a new connection to a server */
	ret = client_connect(peer, addr, port, NULL, NULL, &conn);
	if (ret)
		goto err_mr_dereg;

	/* obtain the remote memory description */
	struct rpma_conn_private_data pdata;
	ret = rpma_conn_get_private_data(conn, &pdata);
	if (ret)
		goto err_conn_disconnect;
	if (pdata.len < sizeof(struct common_data)) {
		fprintf(stderr,
			"Received connection's private data is too small (%u < %zu)\n",
			pdata.len, sizeof(struct common_data));
		ret = -1;
		goto err_conn_disconnect;
	}

	struct common_data *dst_data = pdata.ptr;
	ret = rpma_mr_remote_from_descriptor(&dst_data->descriptors[0],
			dst_data->mr_desc_size, &dst_mr);
	if (ret)
		goto err_conn_disconnect;

	/* write a message with immediate data to the server */
	memcpy(src, (uint32_t *)&imm, sizeof(uint32_t));
	fprintf(stdout, "Writing value '%u' with immediate data '%u'\n",
		*src, (uint32_t)imm);
	ret = rpma_write_with_imm(conn, dst_mr, dst_data->data_offset, src_mr,
			0, KILOBYTE, RPMA_F_COMPLETION_ALWAYS, (uint32_t)imm,
			NULL);
	if (ret)
		goto err_mr_remote_delete;

	/* get the connection's main CQ */
	struct rpma_cq *cq = NULL;
	ret = rpma_conn_get_cq(conn, &cq);
	if (ret)
		goto err_mr_remote_delete;

	/* prepare completions, get one and validate it */
	ret = rpma_cq_wait(cq);
	if (ret)
		goto err_mr_remote_delete;

	ret = rpma_cq_get_wc(cq, 1, &wc, NULL);
	if (ret)
		goto err_mr_remote_delete;

	if (wc.status != IBV_WC_SUCCESS) {
		fprintf(stderr,
			"Received unexpected completion: %s\n",
			ibv_wc_status_str(wc.status));
		ret = -1;
		goto err_mr_remote_delete;
	}

	if (wc.opcode != IBV_WC_RDMA_WRITE) {
		fprintf(stderr,
			"Received unexpected type of operation (%d != %d)\n",
			wc.opcode, IBV_WC_RDMA_WRITE);
		ret = -1;
	}

err_mr_remote_delete:
	/* delete the remote memory region's structure */
	(void) rpma_mr_remote_delete(&dst_mr);

err_conn_disconnect:
	(void) common_disconnect_and_wait_for_conn_close(&conn);

err_mr_dereg:
	/* deregister the memory regions */
	(void) rpma_mr_dereg(&src_mr);

err_peer_delete:
	/* delete the peer object */
	(void) rpma_peer_delete(&peer);

err_mr_free:
	/* free the memory */
	free(src);

	return ret;
}
