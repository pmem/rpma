// SPDX-License-Identifier: BSD-3-Clause
/* Copyright (c) 2020 Fujitsu */
/* Copyright 2021, Intel Corporation */

/*
 * client.c -- a client of the send-with-imm example
 *
 * Please see README.md for a detailed description of this example.
 */

#include <librpma.h>
#include <limits.h>
#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>

#include "common-conn.h"

#define USAGE_STR "usage: %s <server_address> <port> <imm> [word]\n"

int
main(int argc, char *argv[])
{
	/* validate parameters */
	if (argc < 4) {
		fprintf(stderr, USAGE_STR, argv[0]);
		return -1;
	}

	/* configure logging thresholds to see more details */
	rpma_log_set_threshold(RPMA_LOG_THRESHOLD, RPMA_LOG_LEVEL_INFO);
	rpma_log_set_threshold(RPMA_LOG_THRESHOLD_AUX, RPMA_LOG_LEVEL_INFO);

	/* read common parameters */
	char *addr = argv[1];
	char *port = argv[2];
	char *word;
	if (argc == 5)
		word = argv[4];

	uint64_t imm = strtoul(argv[3], NULL, 10);
	if (imm == ULONG_MAX && errno == ERANGE) {
		fprintf(stderr, "strtoul() overflowed\n");
		return -1;
	}

	if (imm > UINT32_MAX) {
		fprintf(stderr,
			"the provided immediate data is too big(%lu > %u)\n",
			imm, UINT32_MAX);
		return -1;
	}

	/* RPMA resources - general */
	struct rpma_peer *peer = NULL;
	struct rpma_conn *conn = NULL;
	struct rpma_mr_local *send_mr = NULL;
	struct rpma_completion cmpl;
	int ret;

	/* prepare memory */
	char *send = malloc_aligned(KILOBYTE);
	if (!send)
		return -1;

	/*
	 * lookup an ibv_context via the address and create a new peer using it
	 */
	ret = client_peer_via_address(addr, &peer);
	if (ret)
		goto err_mr_free;

	if (word) {
		/* register the memory */
		ret = rpma_mr_reg(peer, send, KILOBYTE,
				RPMA_MR_USAGE_SEND, &send_mr);
		if (ret)
			goto err_peer_delete;
	}

	/*
	 * establish a new connection to a server and send an immediate
	 * data for validation on the sever side
	 */
	struct rpma_conn_private_data pdata;
	pdata.ptr = (uint32_t *)&imm;
	pdata.len = sizeof(uint32_t);
	ret = client_connect(peer, addr, port, &pdata, &conn);
	if (ret)
		goto err_mr_dereg;

	if (word) {
		/* send a message with immediate data to the server */
		fprintf(stdout, "send a value %s with immediate data %u\n",
			word, (uint32_t)imm);
		strcpy(send, word);
		ret = rpma_send_with_imm(conn, send_mr, 0, KILOBYTE,
			RPMA_F_COMPLETION_ALWAYS, (uint32_t)imm, NULL);
	} else {
		/* send a 0B message with immediate data to the server */
		fprintf(stdout, "send immediate data %u\n", (uint32_t)imm);
		ret = rpma_send_with_imm(conn, NULL, 0, 0,
			RPMA_F_COMPLETION_ALWAYS, (uint32_t)imm, NULL);
	}
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
			"an unexpected completion: %s\n",
			ibv_wc_status_str(cmpl.op_status));
		ret = -1;
		goto err_conn_disconnect;
	}

	if (cmpl.op != RPMA_OP_SEND) {
		fprintf(stderr,
			"an unexpected type of operation (%d != %d)\n",
			cmpl.op, RPMA_OP_SEND);
		ret = -1;
	}

err_conn_disconnect:
	common_disconnect_and_wait_for_conn_close(&conn);

err_mr_dereg:
	/* deregister the memory regions */
	(void) rpma_mr_dereg(&send_mr);

err_peer_delete:
	/* delete the peer object */
	rpma_peer_delete(&peer);

err_mr_free:
	/* free the memory */
	free(send);

	return ret;
}
