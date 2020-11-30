// SPDX-License-Identifier: BSD-3-Clause
/* Copyright (c) 2020 Fujitsu */

/*
 * server.c -- a server of the send-with-imm example
 *
 * Please see README.md for a detailed description of this example.
 */

#include <inttypes.h>
#include <librpma.h>
#include <stdlib.h>
#include <stdio.h>

#include "common-conn.h"

#define USAGE_STR "usage: %s <server_address> <port>\n"

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

	/* prepare memory */
	char *recv = malloc_aligned(KILOBYTE);
	if (!recv)
		return -1;

	/* RPMA resources */
	struct rpma_peer *peer = NULL;
	struct rpma_mr_local *recv_mr;
	struct rpma_ep *ep = NULL;
	struct rpma_conn *conn = NULL;
	struct rpma_completion cmpl;
	int ret;

	/*
	 * lookup an ibv_context via the address and create a new peer using it
	 */
	ret = server_peer_via_address(addr, &peer);
	if (ret)
		goto err_mr_free;

	/* register the memory */
	ret = rpma_mr_reg(peer, recv, KILOBYTE, RPMA_MR_USAGE_RECV, &recv_mr);
	if (ret)
		goto err_peer_delete;

	/* start a listening endpoint at addr:port */
	ret = rpma_ep_listen(peer, addr, port, &ep);
	if (ret)
		goto err_mr_dereg;

	/*
	 * wait for an incoming connection request, accept it
	 * and wait for its establishment.
	 */
	ret = server_accept_connection(ep, NULL, &conn);
	if (ret)
		goto err_ep_shutdown;

	/* obtain imm_data from conn_param */
	struct rpma_conn_private_data pdata;
	ret = rpma_conn_get_private_data(conn, &pdata);
	if (ret)
		goto err_conn_disconnect;
	uint32_t *data = pdata.ptr;

	/* receice a message with imm_data from the client */
	ret = rpma_recv(conn, recv_mr, 0, KILOBYTE, NULL);
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
			"server got the unsuccessful completion of an operation\n");
		ret = -1;
		goto err_conn_disconnect;
	}

	if (cmpl.op != RPMA_OP_RECV) {
		fprintf(stderr,
			"server got the unexpected type of an operation\n");
		ret = -1;
		goto err_conn_disconnect;
	}

	if (!(cmpl.flags & IBV_WC_WITH_IMM)) {
		fprintf(stderr,
			"server didn't get IBV_WC_WITH_IMM flag\n");
		ret = -1;
		goto err_conn_disconnect;
	}

	if (cmpl.imm_data != *data) {
		fprintf(stderr,
			"server got wrong imm_data %u\n", *data);
		ret = -1;
	} else {
		printf("receive value %s with imm_data %u\n", recv,
			cmpl.imm_data);
	}

err_conn_disconnect:
	common_wait_for_conn_close_and_disconnect(&conn);

err_ep_shutdown:
	rpma_ep_shutdown(&ep);

err_mr_dereg:
	/* deregister the memory regions */
	rpma_mr_dereg(&recv_mr);

err_peer_delete:
	/* delete the peer object */
	rpma_peer_delete(&peer);

err_mr_free:
	/* free the memory */
	free(recv);

	return ret;
}
