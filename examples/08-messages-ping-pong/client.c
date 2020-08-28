// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020, Intel Corporation */

/*
 * client.c -- a client of the messages-ping-pong example
 *
 * Please see README.md for a detailed description of this example.
 */

#include <librpma.h>
#include <librpma_log.h>
#include <limits.h>
#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

#include "common-conn.h"
#include "common-epoll.h"
#include "messages-ping-pong-common.h"

/* Both buffers are allocated one after another. */
#define RECV_OFFSET	0
#define SEND_OFFSET	MSG_SIZE

#define USAGE_STR "usage: %s <server_address> <port> <seed> <rounds>\n"

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
	uint64_t cntr = strtoul_noerror(argv[3]);
	uint64_t rounds = strtoul_noerror(argv[4]);

	int ret = 0, ret2;

	/* RPMA resources - general */
	struct rpma_peer *peer = NULL;
	struct rpma_conn *conn = NULL;

	/* resources - memory region */
	void *mr_ptr = NULL;
	uint64_t *recv_ptr, *send_ptr;
	struct rpma_mr_local *mr = NULL;

	/*
	 * lookup an ibv_context via the address and create a new peer using it
	 */
	ret = client_peer_via_address(addr, &peer);
	if (ret)
		return ret;

	/* allocate a memory */
	mr_ptr = malloc_aligned(MSG_SIZE * 2);
	if (mr_ptr == NULL) {
		ret = -1;
		goto err_peer_delete;
	}

	/* initialize pointers and buffers value */
	recv_ptr = (uint64_t *)((char *)mr_ptr + RECV_OFFSET);
	send_ptr = (uint64_t *)((char *)mr_ptr + SEND_OFFSET);
	*send_ptr = cntr;

	/* register the memory */
	ret = rpma_mr_reg(peer, mr_ptr, MSG_SIZE,
			RPMA_MR_USAGE_RECV | RPMA_MR_USAGE_SEND,
			RPMA_MR_PLT_VOLATILE, &mr);
	if (ret)
		goto err_mr_free;

	/* establish a new connection to a server listening at addr:port */
	ret = client_connect(peer, addr, port, NULL, &conn);
	if (ret)
		goto err_mr_dereg;

	struct rpma_completion cmpl;
	for (; rounds >= 0; --rounds) {
		/* prepare a receive for the server's response */
		if ((ret = rpma_recv(conn, mr, RECV_OFFSET, MSG_SIZE,
				RPMA_F_COMPLETION_ALWAYS, NULL)))
			break;

		/* send a message to the server */
		if ((ret = rpma_send(conn, mr, SEND_OFFSET, MSG_SIZE,
				RPMA_F_COMPLETION_ON_ERROR, NULL)))
			break;

		/* wait for a response */
		if ((ret = rpma_conn_prepare_completions(conn)))
			break;

		/* read the completion and validate its attributes */
		if ((ret = rpma_conn_next_completion(conn, &cmpl)))
			break;

		/* validate the received completion */
		if (cmpl.op_status != IBV_WC_SUCCESS ||
				cmpl.byte_len != MSG_SIZE) {
			(void) fprintf(stderr,
					"received completion is not as expected (%d != %d || %d != %ld)\n",
					cmpl.op_status, IBV_WC_SUCCESS,
					cmpl.byte_len, MSG_SIZE);
			ret = -1;
			break;
		}

		/* copy the new value of the counter and print it out */
		cntr = *recv_ptr;
		printf("%" PRIu64 "\n", cntr);
	}

	ret2 = common_disconnect_and_wait_for_conn_close(&conn);
	if (ret2 && !ret)
		ret = ret2;

err_mr_dereg:
	/* deregister the memory region */
	(void) rpma_mr_dereg(&mr);

err_mr_free:
	/* free the memory */
	free(mr_ptr);

err_peer_delete:
	/* delete the peer */
	(void) rpma_peer_delete(&peer);

	return ret;
}
