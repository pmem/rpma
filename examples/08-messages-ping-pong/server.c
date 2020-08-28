// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020, Intel Corporation */

/*
 * server.c -- a server of the messages-ping-pong example
 *
 * Please see README.md for a detailed description of this example.
 */

#include <inttypes.h>
#include <librpma.h>
#include <librpma_log.h>
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
	int ret = 0, ret2;

	/* prepare memory */
	size_t mr_size = MSG_SIZE * 2;
	struct rpma_mr_local *mr = NULL;
	void *mr_ptr = malloc_aligned(mr_size);
	if (mr_ptr == NULL)
		return -1;
	uint64_t *recv = (uint64_t *)((char *)mr_ptr + RECV_OFFSET);
	uint64_t *send = (uint64_t *)((char *)mr_ptr + SEND_OFFSET);

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
	if ((ret = rpma_mr_reg(peer, mr_ptr, mr_size,
			RPMA_MR_USAGE_RECV | RPMA_MR_USAGE_SEND,
			RPMA_MR_PLT_VOLATILE, &mr)))
		goto err_ep_shutdown;

	/* receive an incoming connection request */
	if ((ret = rpma_ep_next_conn_req(ep, &req)))
		goto err_mr_dereg;

	/*
	 * Put an initial receive to be prepared for the first message of
	 * the client's ping-pong.
	 */
	if ((ret = rpma_conn_req_recv(req, mr, RECV_OFFSET, MSG_SIZE,
			RPMA_F_COMPLETION_ALWAYS, recv))) {
		(void) rpma_conn_req_delete(&req);
		goto err_mr_dereg;
	}

	/* accept the connection request and obtain the connection object */
	if ((ret = rpma_conn_req_connect(&req, NULL, &conn)))
		goto err_ep_shutdown;

	/* wait for the connection to be established */
	if ((ret = rpma_conn_next_event(conn, &conn_event)))
		goto err_ep_shutdown;
	if (conn_event != RPMA_CONN_ESTABLISHED) {
		fprintf(stderr,
				"rpma_conn_next_event returned an unexptected event\n");
		goto err_ep_shutdown;
	}

	while (1) {
		/* prepare completions, get one and validate it */
		if ((ret = rpma_conn_prepare_completions(conn))) {
			break;
		} else if ((ret = rpma_conn_next_completion(conn, &cmpl))) {
			break;
		} else if (cmpl.op != RPMA_OP_RECV ||
				cmpl.op_status != IBV_WC_SUCCESS ||
				cmpl.op_context != recv ||
				cmpl.byte_len != MSG_SIZE) {
			(void) fprintf(stderr,
					"received completion is not as expected (%d != %d || %d != %d || %p != %p || %"
					PRIu32 " != %ld)\n",
					cmpl.op, RPMA_OP_RECV,
					cmpl.op_status, IBV_WC_SUCCESS,
					cmpl.op_context, recv,
					cmpl.byte_len, MSG_SIZE);
			break;
		}

		if (*recv == I_M_DONE)
			break;

		/* print the received old value of the client's counter */
		printf("%" PRIu64 "\n", *recv);

		/* calculate a new counter's value */
		*send = *recv + 1;

		/* prepare a receive for the client's response */
		if ((ret = rpma_recv(conn, mr, RECV_OFFSET, MSG_SIZE,
				RPMA_F_COMPLETION_ALWAYS, recv)))
			break;

		/* send the new value to the client */
		if ((ret = rpma_send(conn, mr, SEND_OFFSET, MSG_SIZE,
				RPMA_F_COMPLETION_ON_ERROR, NULL)))
			break;
	}

	if ((ret2 = common_disconnect_and_wait_for_conn_close(&conn)) && !ret)
		ret = ret2;

err_mr_dereg:
	/* deregister the memory region */
	if ((ret2 = rpma_mr_dereg(&mr)) && !ret)
		ret = ret2;

err_ep_shutdown:
	if ((ret2 = rpma_ep_shutdown(&ep)) && !ret)
		ret = ret2;

err_peer_delete:
	/* delete the peer object */
	if ((ret2 = rpma_peer_delete(&peer)) && !ret)
		ret = ret2;

err_free:
	free(mr_ptr);

	return ret;
}
