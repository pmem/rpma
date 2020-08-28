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
#include "messages-ping-pong-common.h"

#define USAGE_STR "usage: %s <server_address> <port> <seed> <rounds> " \
		"[<sleep>]\n"

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
	uint64_t sleep_usec = 0;

	if (argc == 5)
		sleep_usec = strtoul_noerror(argv[5]);

	int ret = 0, ret2;

	/* RPMA resources - general */
	struct rpma_peer *peer = NULL;
	struct rpma_conn *conn = NULL;
	struct rpma_completion cmpl;

	/*
	 * lookup an ibv_context via the address and create a new peer using it
	 */
	if ((ret = client_peer_via_address(addr, &peer)))
		return ret;

	/* prepare memory */
	size_t mr_size = MSG_SIZE * 2;
	struct rpma_mr_local *mr = NULL;
	void *mr_ptr = malloc_aligned(mr_size);
	if (mr_ptr == NULL) {
		ret = -1;
		goto err_peer_delete;
	}
	uint64_t *recv = (uint64_t *)((char *)mr_ptr + RECV_OFFSET);
	uint64_t *send = (uint64_t *)((char *)mr_ptr + SEND_OFFSET);

	/* register the memory */
	if ((ret = rpma_mr_reg(peer, mr_ptr, mr_size,
			RPMA_MR_USAGE_RECV | RPMA_MR_USAGE_SEND,
			RPMA_MR_PLT_VOLATILE, &mr)))
		goto err_mr_free;

	/* establish a new connection to a server listening at addr:port */
	if ((ret = client_connect(peer, addr, port, NULL, &conn)))
		goto err_mr_dereg;

	while (--rounds) {
		/* prepare a receive for the server's response */
		if ((ret = rpma_recv(conn, mr, RECV_OFFSET, MSG_SIZE,
				RPMA_F_COMPLETION_ALWAYS, recv)))
			break;

		/* send a message to the server */
		*send = cntr;
		if ((ret = rpma_send(conn, mr, SEND_OFFSET, MSG_SIZE,
				RPMA_F_COMPLETION_ON_ERROR, NULL)))
			break;

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
					"received completion is not as expected (%d != %d || %p != %p || %"
					PRIu32 " != %ld)\n",
					cmpl.op_status, IBV_WC_SUCCESS,
					cmpl.op_context, recv,
					cmpl.byte_len, MSG_SIZE);
			ret = -1;
			break;
		}

		/* copy the new value of the counter and print it out */
		cntr = *recv;
		printf("%" PRIu64 "\n", cntr);

		/* sleep if required */
		if (sleep_usec > 0)
			(void) usleep(sleep_usec);
	}

	/* send the I_M_DONE message */
	*send = I_M_DONE;
	if ((ret2 = rpma_send(conn, mr, SEND_OFFSET, MSG_SIZE,
			RPMA_F_COMPLETION_ON_ERROR, NULL)) && !ret)
		ret = ret2;

	/* XXX is waiting for RDMA.send completion required? */

	if ((ret2 = common_disconnect_and_wait_for_conn_close(&conn)) && !ret)
		ret = ret2;

err_mr_dereg:
	/* deregister the memory region */
	if ((ret2 = rpma_mr_dereg(&mr)) && !ret)
		ret = ret2;

err_mr_free:
	/* free the memory */
	free(mr_ptr);

err_peer_delete:
	/* delete the peer object */
	if ((ret2 = rpma_peer_delete(&peer)) && !ret)
		ret = ret2;

	return ret;
}
