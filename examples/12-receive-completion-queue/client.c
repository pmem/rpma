// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2022, Intel Corporation */

/*
 * client.c -- a client of the receive completion queue example
 *
 * Please see README.md for a detailed description of this example.
 */

#include <librpma.h>
#include <limits.h>
#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

#include "common-conn.h"
#include "receive-completion-queue-common.h"

#define USAGE_STR \
		"usage: %s <server_address> <port> <start_value> <rounds> [<sleep>]\n"

static uint64_t
strtoul_noerror(const char *in)
{
	uint64_t out = strtoul(in, NULL, 10);
	if (out == ULONG_MAX && errno == ERANGE) {
		(void) fprintf(stderr, "strtoul(%s) overflowed\n", in);
		exit(-1);
	}
	return out;
}

int
main(int argc, char *argv[])
{
	/* validate parameters */
	if (argc < 5) {
		fprintf(stderr, USAGE_STR, argv[0]);
		exit(-1);
	}

	/* configure logging thresholds to see more details */
	rpma_log_set_threshold(RPMA_LOG_THRESHOLD, RPMA_LOG_LEVEL_INFO);
	rpma_log_set_threshold(RPMA_LOG_THRESHOLD_AUX, RPMA_LOG_LEVEL_INFO);

	/* read common parameters */
	char *addr = argv[1];
	char *port = argv[2];
	uint64_t counter = strtoul_noerror(argv[3]);
	uint64_t rounds = strtoul_noerror(argv[4]);
	uint64_t sleep_usec = 0;

	if (argc >= 6)
		sleep_usec = strtoul_noerror(argv[5]);

	int ret;

	/* RPMA resources - general */
	struct rpma_peer *peer = NULL;
	struct rpma_conn *conn = NULL;

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

	/*
	 * lookup an ibv_context via the address and create a new peer using it
	 */
	if ((ret = client_peer_via_address(addr, &peer)))
		goto err_mr_free;

	/* register the memory */
	if ((ret = rpma_mr_reg(peer, recv, MSG_SIZE, RPMA_MR_USAGE_RECV,
				&recv_mr)))
		goto err_peer_delete;
	if ((ret = rpma_mr_reg(peer, send, MSG_SIZE, RPMA_MR_USAGE_SEND,
				&send_mr))) {
		(void) rpma_mr_dereg(&recv_mr);
		goto err_peer_delete;
	}

	/* create a new connection configuration and set RCQ size */
	struct rpma_conn_cfg *cfg = NULL;
	if ((ret = rpma_conn_cfg_new(&cfg)))
		goto err_mr_dereg;

	if ((ret = rpma_conn_cfg_set_rcq_size(cfg, RCQ_SIZE)))
		goto err_cfg_delete;

	/* establish a new connection to a server listening at addr:port */
	if ((ret = client_connect(peer, addr, port, cfg, NULL, &conn)))
		goto err_cfg_delete;

	/* get the connection's main CQ */
	struct rpma_cq *cq = NULL;
	if ((ret = rpma_conn_get_cq(conn, &cq)))
		goto err_conn_disconnect;

	/* get the connection's RCQ */
	struct rpma_cq *rcq = NULL;
	if ((ret = rpma_conn_get_rcq(conn, &rcq)))
		goto err_conn_disconnect;

	while (--rounds) {
		/* prepare a receive for the server's response */
		if ((ret = rpma_recv(conn, recv_mr, 0, MSG_SIZE, recv)))
			break;

		/* send a message to the server */
		(void) printf("CLIENT: Value sent: %" PRIu64 "\n", counter);
		*send = counter;
		if ((ret = rpma_send(conn, send_mr, 0, MSG_SIZE,
				RPMA_F_COMPLETION_ALWAYS, NULL)))
			break;

		/* get one send completion and validate it */
		if ((ret = get_wc_and_validate(cq, IBV_WC_SEND, "rpma_send()")))
			break;

		/* get one receive completion and validate it */
		if ((ret = get_wc_and_validate(rcq, IBV_WC_RECV,
				"rpma_recv()")))
			break;

		/* copy the new value of the counter and print it out */
		counter = *recv;
		printf("CLIENT: Value received: %" PRIu64 "\n", counter);

		/* sleep if required */
		if (sleep_usec > 0)
			(void) usleep(sleep_usec);
	}

	/* send the I_M_DONE message */
	*send = I_M_DONE;
	ret |= rpma_send(conn, send_mr, 0, MSG_SIZE, RPMA_F_COMPLETION_ON_ERROR,
			NULL);

err_conn_disconnect:
	ret |= common_disconnect_and_wait_for_conn_close(&conn);

err_cfg_delete:
	ret |= rpma_conn_cfg_delete(&cfg);

err_mr_dereg:
	/* deregister the memory regions */
	ret |= rpma_mr_dereg(&send_mr);
	ret |= rpma_mr_dereg(&recv_mr);

err_peer_delete:
	/* delete the peer object */
	ret |= rpma_peer_delete(&peer);

err_mr_free:
	/* free the memory */
	free(send);
	free(recv);

	return ret ? -1 : 0;
}
