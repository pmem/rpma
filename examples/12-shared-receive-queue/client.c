// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020, Intel Corporation */

/*
 * client.c -- a client of multiple connection example using SRQ
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
#include "shared-receive-queue.h"

static char *Names[] = {
	"Andy",
	"Chet",
	"Derek",
	"Janek",
	"Lukasz",
	"Oksana",
	"Pawel",
	"Piotr",
	"Tomasz",
	"Xiang",
	"Xiaoran",
	"Xiaoyan"
};

#define NAMES_NUM (sizeof(Names) / sizeof(Names[0]))

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
	int send = 0, recv = 0;

	/* RPMA resources - general */
	struct rpma_peer *peer = NULL;
	struct rpma_conn *conn = NULL;

	/* resources - memory region */
	void *send_ptr, *recv_ptr = NULL;
	struct rpma_mr_local *send_mr, *recv_mr = NULL;
	struct rpma_completion cmpl;

	/* allocate memory region */
	send_ptr = malloc_aligned(MAX_MSG_SIZE);
	if (send_ptr == NULL)
		return -1;
	recv_ptr = malloc_aligned(MAX_MSG_SIZE);
	if (recv_ptr == NULL) {
		free(send_ptr);
		return -1;
	}

	/*
	 * lookup an ibv_context via the address and create a new peer using it
	 */
	if ((ret = client_peer_via_address(addr, &peer)))
		goto err_free;

	/* register the memory */
	if ((ret = rpma_mr_reg(peer, send_ptr, MAX_MSG_SIZE, RPMA_MR_USAGE_SEND,
				&send_mr)))
		goto err_peer_delete;
	if ((ret = rpma_mr_reg(peer, recv_ptr, MAX_MSG_SIZE, RPMA_MR_USAGE_RECV,
				&recv_mr))) {
		(void) rpma_mr_dereg(&send_mr);
		goto err_peer_delete;
	}

	/* establish a new connection to a server listening at addr:port */
	if ((ret = client_connect(peer, addr, port, NULL, &conn)))
		goto err_mr_dereg;

	while (!(send == 2 && recv == 2)) {
		if (recv == 0) {
			/* Receive server's reply */
			if (rpma_recv(conn, recv_mr, 0, MAX_MSG_SIZE, NULL))
				goto err_conn_disconnect;
			recv++;
		}
		if (send == 0) {
			/* Say hello to server */
			sprintf((char *)send_ptr, "hello");
			if (rpma_send(conn, send_mr, 0, MAX_MSG_SIZE,
						RPMA_F_COMPLETION_ALWAYS, NULL))
				goto err_conn_disconnect;
			send++;
		}
		if (rpma_conn_completion_wait(conn))
			goto err_conn_disconnect;
		if (rpma_conn_completion_get(conn, &cmpl))
			goto err_conn_disconnect;
		if (cmpl.op == RPMA_OP_SEND &&
			cmpl.op_status == IBV_WC_SUCCESS)
			send++;
		else if (cmpl.op == RPMA_OP_RECV &&
			cmpl.op_status == IBV_WC_SUCCESS)
			recv++;
		else {
			printf("Fail to communicate with server: "
				"op=%d status=%d\n", cmpl.op, cmpl.op_status);
			goto err_conn_disconnect;
		}
	}

	printf("My pid is %d: Server says: %s\n", getpid(), (char *)recv_ptr);

	/* Send my name to server */
	/* pick a name */
	srand((unsigned int)getpid());
	const char *name = Names[(long unsigned int)rand() % NAMES_NUM];
	(void) strncpy((char *)send_ptr, name, (MAX_MSG_SIZE - 1));

	if (rpma_send(conn, send_mr, 0, MAX_MSG_SIZE,
				RPMA_F_COMPLETION_ALWAYS, NULL))
		goto err_conn_disconnect;
	if (rpma_conn_completion_wait(conn))
		goto err_conn_disconnect;
	if (rpma_conn_completion_get(conn, &cmpl))
		goto err_conn_disconnect;
	if (cmpl.op != RPMA_OP_SEND || cmpl.op_status != IBV_WC_SUCCESS)
		printf("Fail to send name to server: op=%d status=%d\n",
						cmpl.op, cmpl.op_status);

err_conn_disconnect:
	ret = common_wait_for_conn_close_and_disconnect(&conn);

err_mr_dereg:
	/* deregister the memory region */
	(void) rpma_mr_dereg(&send_mr);
	(void) rpma_mr_dereg(&recv_mr);


err_peer_delete:
	/* delete the peer */
	(void) rpma_peer_delete(&peer);

err_free:
	/* free the memory */
	free(send_ptr);
	free(recv_ptr);

	return ret;
}
