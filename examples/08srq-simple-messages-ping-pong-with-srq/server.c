// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2022, Intel Corporation */

/*
 * server.c -- a server of the simple-messages-ping-pong-with-srq example
 *
 * Please see README.md for a detailed description of this example.
 */

#include <inttypes.h>
#include <librpma.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/epoll.h>

#include "common-conn.h"
#include "common-messages-ping-pong.h"
#include "common-epoll.h"

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
	int ret;

	/* prepare memory */
	struct rpma_mr_local *recv_mr, *send_mr;
	uint64_t *recv = malloc_aligned(MSG_SIZE);
	if (recv == NULL)
		return -1;

	uint64_t *send = malloc_aligned(MSG_SIZE);
	if (send == NULL) {
		ret = -1;
		goto err_free_recv;
	}

	/* RPMA resources */
	struct rpma_peer *peer = NULL;
	struct rpma_srq *srq = NULL;
	struct rpma_conn *conn = NULL;
	struct rpma_conn_cfg *cfg = NULL;
	struct rpma_ep *ep = NULL;
	struct rpma_cq *rcq = NULL;
	int num_got = 0;
	uint32_t qp_num;
	struct ibv_wc wc;

	/*
	 * lookup an ibv_context via the address and create a new peer using it
	 */
	if ((ret = server_peer_via_address(addr, &peer)))
		goto err_free_send;

	/* create a shared RQ object */
	if ((ret = rpma_srq_new(peer, NULL, &srq)))
		goto err_peer_delete;

	/* create a new connection configuration */
	if ((ret = rpma_conn_cfg_new(&cfg)))
		goto err_srq_delete;

	/* set the shared RQ object for the connection configuration */
	if ((ret = rpma_conn_cfg_set_srq(cfg, srq)))
		goto err_conn_cfg_delete;

	/* start a listening endpoint at addr:port */
	if ((ret = rpma_ep_listen(peer, addr, port, &ep)))
		goto err_conn_cfg_delete;

	/* register the memory */
	if ((ret = rpma_mr_reg(peer, recv, MSG_SIZE, RPMA_MR_USAGE_RECV, &recv_mr)))
		goto err_ep_shutdown;

	if ((ret = rpma_mr_reg(peer, send, MSG_SIZE, RPMA_MR_USAGE_SEND, &send_mr)))
		goto err_recv_mr_dereg;

	/*
	 * Put an initial receive for a connection to be prepared for the first message
	 * of the client's ping-pong.
	 */
	if ((ret = rpma_srq_recv(srq, recv_mr, 0, MSG_SIZE, recv)))
		goto err_conn_disconnect;

	/*
	 * Wait for an incoming connection request, accept it and wait for its establishment.
	 */
	if ((ret = server_accept_connection(ep, cfg, NULL, &conn)))
		goto err_conn_disconnect;

	/* get the qp_num of the connection */
	if ((ret = rpma_conn_get_qp_num(conn, &qp_num)))
		goto err_conn_disconnect;

	/* get the receive CQ of the rpma_srq object */
	if ((ret = rpma_srq_get_rcq(srq, &rcq)))
		goto err_conn_disconnect;

	int recv_cmpl = 0;

	while (1) {
		do {
			/* wait for the receive completion to be ready */
			if ((ret = rpma_cq_wait(rcq)))
				goto err_conn_disconnect;

			/* reset num_got to 0  */
			num_got = 0;

			/* get the next recv completion */
			if ((ret = rpma_cq_get_wc(rcq, 1, &wc, &num_got)))
				/* lack of completion is not an error */
				if (ret != RPMA_E_NO_COMPLETION)
					goto err_conn_disconnect;

			if (num_got) {
				/* validate the received completion */
				int send_cmpl; /* not used */
				ret = validate_wc(&wc, recv, &send_cmpl, &recv_cmpl);
				if (ret)
					goto err_conn_disconnect;
			}

		} while (recv_cmpl == 0);

		if (*recv == I_M_DONE)
			break;

		if (qp_num != wc.qp_num) {
			ret = -1;
			(void) fprintf(stderr,
				"Error: number of QP (qp_num) in the received completion "
				"(%" PRIu32 ") differs from the one of the connection "
				"(%" PRIu32 ")\n",
				wc.qp_num, qp_num);
			break;
		}

		/* print the received old value of the client's counter */
		(void) printf("Value received: %" PRIu64 "\n", *recv);

		*send = *recv + 1;

		/* prepare a receive for the client's request */
		if ((ret = rpma_srq_recv(srq, recv_mr, 0, MSG_SIZE, recv)))
			break;

		/* send the new value to the client */
		(void) printf("Value sent: %" PRIu64 "\n", *send);

		/* send a message to the client */
		ret = rpma_send(conn, send_mr, 0, MSG_SIZE, RPMA_F_COMPLETION_ON_ERROR, NULL);
		if (ret)
			break;
	}


err_conn_disconnect:
	/* disconnect the client */
	(void) rpma_conn_disconnect(conn);
	(void) rpma_conn_delete(&conn);

	ret |= rpma_mr_dereg(&send_mr);

err_recv_mr_dereg:
	ret |= rpma_mr_dereg(&recv_mr);

err_ep_shutdown:
	ret |= rpma_ep_shutdown(&ep);

err_conn_cfg_delete:
	ret |= rpma_conn_cfg_delete(&cfg);

err_srq_delete:
	ret |= rpma_srq_delete(&srq);

err_peer_delete:
	/* delete the peer object */
	ret |= rpma_peer_delete(&peer);

err_free_send:
	free(send);

err_free_recv:
	free(recv);

	return ret ? -1 : 0;
}
