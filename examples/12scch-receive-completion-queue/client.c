// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2022, Intel Corporation */

/*
 * client.c -- a client of the receive completion queue example
 *
 * Please see README.md for a detailed description of this example.
 */

#include <librpma.h>
#include <inttypes.h>
#include <stdlib.h>
#include <unistd.h>

#include "common-conn.h"
#include "common-utils.h"
#include "receive-completion-queue-common.h"

#define USAGE_STR "usage: %s <server_address> <port> <start_value> <rounds> [<sleep>]\n"

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
	struct ibv_wc wc;

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
	ret = client_peer_via_address(addr, &peer);
	if (ret)
		goto err_mr_free;

	/* register the memory */
	ret = rpma_mr_reg(peer, recv, MSG_SIZE, RPMA_MR_USAGE_RECV, &recv_mr);
	if (ret)
		goto err_peer_delete;
	ret = rpma_mr_reg(peer, send, MSG_SIZE, RPMA_MR_USAGE_SEND, &send_mr);
	if (ret) {
		(void) rpma_mr_dereg(&recv_mr);
		goto err_peer_delete;
	}

	/* create a new connection configuration and set RCQ size */
	struct rpma_conn_cfg *cfg = NULL;
	ret = rpma_conn_cfg_new(&cfg);
	if (ret)
		goto err_mr_dereg;

	ret = rpma_conn_cfg_set_rcq_size(cfg, RCQ_SIZE);
	if (ret)
		goto err_cfg_delete;

	ret = rpma_conn_cfg_set_compl_channel(cfg, true);
	if (ret)
		goto err_cfg_delete;

	/* establish a new connection to a server listening at addr:port */
	ret = client_connect(peer, addr, port, cfg, NULL, &conn);
	if (ret)
		goto err_cfg_delete;

	while (--rounds) {
		/* prepare a receive for the server's response */
		ret = rpma_recv(conn, recv_mr, 0, MSG_SIZE, recv);
		if (ret)
			break;

		/* send a message to the server */
		(void) printf("CLIENT: Value sent: %" PRIu64 "\n", counter);
		*send = counter;
		ret = rpma_send(conn, send_mr, 0, MSG_SIZE, RPMA_F_COMPLETION_ALWAYS, NULL);
		if (ret)
			break;

		/* get one send completion and validate it */
		ret = wait_and_validate_completion(conn, IBV_WC_SEND, &wc);
		if (ret)
			break;

		/* get one receive completion and validate it */
		ret = wait_and_validate_completion(conn, IBV_WC_RECV, &wc);
		if (ret)
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
	ret |= rpma_send(conn, send_mr, 0, MSG_SIZE, RPMA_F_COMPLETION_ON_ERROR, NULL);

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
