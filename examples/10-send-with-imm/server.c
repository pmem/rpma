// SPDX-License-Identifier: BSD-3-Clause
/* Copyright (c) 2020-2021 Fujitsu */
/* Copyright 2021-2022, Intel Corporation */

/*
 * server.c -- a server of the send-with-imm example
 *
 * Please see README.md for a detailed description of this example.
 */

#include <inttypes.h>
#include <librpma.h>
#include <stdlib.h>
#include <stdio.h>
#include <arpa/inet.h>

#include "common-conn.h"

#define USAGE_STR "usage: %s <server_address> <port>\n"

int
main(int argc, char *argv[])
{
	/* validate parameters */
	if (argc < 3) {
		fprintf(stderr, USAGE_STR, argv[0]);
		return -1;
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
	struct rpma_mr_local *recv_mr = NULL;
	struct rpma_ep *ep = NULL;
	struct rpma_conn_req *req = NULL;
	struct rpma_conn *conn = NULL;
	enum rpma_conn_event conn_event = RPMA_CONN_UNDEFINED;
	struct ibv_wc wc;
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

	/* receive an incoming connection request */
	ret = rpma_ep_next_conn_req(ep, NULL, &req);
	if (ret)
		goto err_ep_shutdown;

	/* prepare to receive a message with immediate data from the client */
	ret = rpma_conn_req_recv(req, recv_mr, 0, KILOBYTE, NULL);
	if (ret) {
		rpma_conn_req_delete(&req);
		goto err_ep_shutdown;
	}

	/* accept the connection request and obtain the connection object */
	ret = rpma_conn_req_connect(&req, NULL, &conn);
	if (ret)
		goto err_ep_shutdown;

	/* wait for the connection to be established */
	ret = rpma_conn_next_event(conn, &conn_event);
	if (ret)
		goto err_conn_disconnect;
	if (conn_event != RPMA_CONN_ESTABLISHED) {
		fprintf(stderr,
			"rpma_conn_next_event() returned an unexpected event: %s\n",
			rpma_utils_conn_event_2str(conn_event));
		ret = -1;
		goto err_conn_disconnect;
	}

	/* get the expected immediate data from the connection's private data */
	struct rpma_conn_private_data pdata;
	ret = rpma_conn_get_private_data(conn, &pdata);
	if (ret)
		goto err_conn_disconnect;
	if (pdata.len < sizeof(uint32_t)) {
		fprintf(stderr,
			"Received connection's private data is too small (%u < %zu)\n",
			pdata.len, sizeof(uint32_t));
		ret = -1;
		goto err_conn_disconnect;
	}
	uint32_t *exp_imm = pdata.ptr;

	/* get the connection's main CQ */
	struct rpma_cq *cq = NULL;
	ret = rpma_conn_get_cq(conn, &cq);
	if (ret)
		goto err_conn_disconnect;

	/* prepare completions, get one and validate it */
	ret = rpma_cq_wait(cq);
	if (ret)
		goto err_conn_disconnect;

	ret = rpma_cq_get_wc(cq, 1, &wc, NULL);
	if (ret)
		goto err_conn_disconnect;

	if (wc.status != IBV_WC_SUCCESS) {
		fprintf(stderr, "Received unexpected completion: %s\n",
			ibv_wc_status_str(wc.status));
		ret = -1;
		goto err_conn_disconnect;
	}

	if (wc.opcode != IBV_WC_RECV) {
		fprintf(stderr, "Received unexpected type of operation (%d != %d)\n", wc.opcode,
			IBV_WC_RECV);
		ret = -1;
		goto err_conn_disconnect;
	}

	if (!(wc.wc_flags & IBV_WC_WITH_IMM)) {
		fprintf(stderr, "Received unexpected completion flag (no IBV_WC_WITH_IMM)\n");
		ret = -1;
		goto err_conn_disconnect;
	}

	uint32_t imm = ntohl(wc.imm_data);
	if (imm != *exp_imm) {
		fprintf(stderr, "Received unexpected immediate data (%u != %u)\n", imm, *exp_imm);
		ret = -1;
	} else {
		if (wc.byte_len == 0)
			recv[0] = '\0';
		printf("Received value '%s' with immediate data '%u'\n", recv, imm);
	}

err_conn_disconnect:
	(void) common_wait_for_conn_close_and_disconnect(&conn);

err_ep_shutdown:
	(void) rpma_ep_shutdown(&ep);

err_mr_dereg:
	/* deregister the memory regions */
	(void) rpma_mr_dereg(&recv_mr);

err_peer_delete:
	/* delete the peer object */
	(void) rpma_peer_delete(&peer);

err_mr_free:
	/* free the memory */
	free(recv);

	return ret;
}
