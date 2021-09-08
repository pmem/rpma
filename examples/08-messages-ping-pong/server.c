// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020-2021, Intel Corporation */
/* Copyright 2021, Fujitsu */

/*
 * server.c -- a server of the messages-ping-pong example
 *
 * Please see README.md for a detailed description of this example.
 */

#include <inttypes.h>
#include <librpma.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/epoll.h>
#include <unistd.h>

#define USAGE_STR "usage: %s <server_address> <port>\n"

#include "common-conn.h"
#include "messages-ping-pong-common.h"

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

	/* prepare memory */
	struct rpma_mr_local *recv_mr, *send_mr;
	uint64_t *recv = malloc_aligned(MSG_SIZE);
	if (recv == NULL)
		return -1;
	uint64_t *send = malloc_aligned(MSG_SIZE);
	if (send == NULL) {
		free(recv);
		return -1;
	}

	/* RPMA resources */
	struct rpma_peer *peer = NULL;
	struct rpma_ep *ep = NULL;
	struct rpma_conn_req *req = NULL;
	enum rpma_conn_event conn_event = RPMA_CONN_UNDEFINED;
	struct rpma_conn *conn = NULL;
	struct rpma_completion cmpl;

	/*
	 * lookup an ibv_context via the address and create a new peer using it
	 */
	if ((ret = server_peer_via_address(addr, &peer)))
		goto err_free;

	/* start a listening endpoint at addr:port */
	if ((ret = rpma_ep_listen(peer, addr, port, &ep)))
		goto err_peer_delete;

	/* register the memory */
	if ((ret = rpma_mr_reg(peer, recv, MSG_SIZE, RPMA_MR_USAGE_RECV,
				&recv_mr)))
		goto err_ep_shutdown;
	if ((ret = rpma_mr_reg(peer, send, MSG_SIZE, RPMA_MR_USAGE_SEND,
				&send_mr))) {
		(void) rpma_mr_dereg(&recv_mr);
		goto err_ep_shutdown;
	}

	/* receive an incoming connection request */
	if ((ret = rpma_ep_next_conn_req(ep, NULL, &req)))
		goto err_mr_dereg;

	/*
	 * Put an initial receive to be prepared for the first message of
	 * the client's ping-pong.
	 */
	if ((ret = rpma_conn_req_recv(req, recv_mr, 0, MSG_SIZE, recv))) {
		(void) rpma_conn_req_delete(&req);
		goto err_mr_dereg;
	}

	/* accept the connection request and obtain the connection object */
	if ((ret = rpma_conn_req_connect(&req, NULL, &conn)))
		goto err_mr_dereg;

	/* wait for the connection to be established */
	if ((ret = rpma_conn_next_event(conn, &conn_event)))
		goto err_conn_disconnect;
	if (conn_event != RPMA_CONN_ESTABLISHED) {
		fprintf(stderr,
			"rpma_conn_next_event returned an unexpected event: %s\n",
			rpma_utils_conn_event_2str(conn_event));
		goto err_conn_disconnect;
	}

	/* get the connection's main CQ */
	struct rpma_cq *cq = NULL;
	if ((ret = rpma_conn_get_cq(conn, &cq)))
		goto err_conn_disconnect;

	/* RPMA_OP_SEND completion in the first round is not present */
	int send_cmpl = 1;
	int recv_cmpl = 0;

	while (1) {
		do {
			/* prepare completions, get one and validate it */
			ret = rpma_cq_get_completion(cq, &cmpl);
			if (ret && ret != RPMA_E_NO_COMPLETION)
				break;

			if (ret == RPMA_E_NO_COMPLETION) {
				if ((ret = rpma_cq_wait(cq))) {
					break;
				} else if ((ret = rpma_cq_get_completion(cq,
						&cmpl))) {
					if (ret == RPMA_E_NO_COMPLETION)
						continue;
					break;
				}
			}

			if (cmpl.op_status != IBV_WC_SUCCESS) {
				(void) fprintf(stderr,
					"rpma_send()/rpma_recv() failed: %s\n",
					ibv_wc_status_str(cmpl.op_status));
				ret = -1;
				break;
			}

			if (cmpl.op == RPMA_OP_SEND) {
				send_cmpl = 1;
			} else if (cmpl.op == RPMA_OP_RECV) {
				if (cmpl.op_context != recv ||
						cmpl.byte_len != MSG_SIZE) {
					(void) fprintf(stderr,
						"received completion is not as expected (%p != %p [cmpl.op_context] || %"
						PRIu32
						" != %ld [cmpl.byte_len] )\n",
						cmpl.op_context, (void *)recv,
						cmpl.byte_len, MSG_SIZE);
					ret = -1;
					break;
				}

				recv_cmpl = 1;
			}
		} while (!send_cmpl || !recv_cmpl);

		if (ret)
			break;

		if (*recv == I_M_DONE)
			break;

		/* print the received old value of the client's counter */
		(void) printf("Value received: %" PRIu64 "\n", *recv);

		/* calculate a new counter's value */
		*send = *recv + 1;

		/* prepare a receive for the client's response */
		if ((ret = rpma_recv(conn, recv_mr, 0, MSG_SIZE, recv)))
			break;

		/* send the new value to the client */
		(void) printf("Value sent: %" PRIu64 "\n", *send);
		if ((ret = rpma_send(conn, send_mr, 0, MSG_SIZE,
				/*
				 * XXX when using RPMA_F_COMPLETION_ON_ERROR
				 * after few rounds rpma_send() returns ENOMEM.
				 */
				RPMA_F_COMPLETION_ALWAYS, NULL)))
			break;

		/* reset */
		send_cmpl = 0;
		recv_cmpl = 0;
	}

err_conn_disconnect:
	ret |= common_disconnect_and_wait_for_conn_close(&conn);

err_mr_dereg:
	/* deregister the memory regions */
	ret |= rpma_mr_dereg(&send_mr);
	ret |= rpma_mr_dereg(&recv_mr);

err_ep_shutdown:
	ret |= rpma_ep_shutdown(&ep);

err_peer_delete:
	/* delete the peer object */
	ret |= rpma_peer_delete(&peer);

err_free:
	free(send);
	free(recv);

	return ret ? -1 : 0;
}
