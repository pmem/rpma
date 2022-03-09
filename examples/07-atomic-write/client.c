// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020-2022, Intel Corporation */
/* Copyright 2021-2022, Fujitsu */

/*
 * client.c -- a client of the atomic-write example
 *
 * Please see README.md for a detailed description of this example.
 */

#include <librpma.h>
#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>
#include "common-conn.h"

#define USAGE_STR "usage: %s <server_address> <port> <word1> [<word2>] [..]\n"

#define FLUSH_ID		(void *)0xF01D /* a random identifier */
#define KILOBYTE		1024
#define MAX_WORD_LENGTH		(KILOBYTE - 1)

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
	int ret;

	/* resources - memory region */
	void *mr_ptr = NULL;
	size_t mr_size = KILOBYTE;
	struct rpma_mr_remote *remote_mr = NULL;
	size_t remote_size = 0;
	size_t dst_used_offset = 0;
	struct rpma_mr_local *local_mr = NULL;
	struct ibv_wc wc;
	union {
		uint64_t uint64;
		char buf[8]; /* atomic write requires exactly 8-bytes buffer */
	} used;

	/* prepare memory */
	mr_ptr = malloc_aligned(KILOBYTE);
	if (mr_ptr == NULL)
		return -1;

	/* RPMA resources */
	struct rpma_peer_cfg *pcfg = NULL;
	struct rpma_peer *peer = NULL;
	struct rpma_conn *conn = NULL;

	/*
	 * lookup an ibv_context via the address and create a new peer using it
	 */
	if ((ret = client_peer_via_address(addr, &peer)))
		goto err_free;

	/* establish a new connection to a server listening at addr:port */
	if ((ret = client_connect(peer, addr, port, NULL, NULL, &conn)))
		goto err_peer_delete;

	/*
	 * Create a remote peer's configuration structure, enable persistent
	 * flush support, and apply it to the current connection. (unilaterally)
	 */
	if ((ret = rpma_peer_cfg_new(&pcfg)))
		goto err_conn_disconnect;
	if ((ret = rpma_peer_cfg_set_direct_write_to_pmem(pcfg, true)))
		goto err_peer_cfg_delete;
	if ((ret = rpma_conn_apply_remote_peer_cfg(conn, pcfg)))
		goto err_peer_cfg_delete;

	/* register the memory for the remote log manipulation */
	if ((ret = rpma_mr_reg(peer, mr_ptr, mr_size,
			RPMA_MR_USAGE_WRITE_SRC | RPMA_MR_USAGE_READ_DST,
			&local_mr)))
		goto err_peer_cfg_delete;

	/* obtain the remote memory description */
	struct rpma_conn_private_data pdata;
	ret = rpma_conn_get_private_data(conn, &pdata);
	if (ret != 0 || pdata.len < sizeof(struct common_data))
		goto err_mr_dereg;

	/*
	 * Create a remote memory registration structure from the received
	 * descriptor.
	 */
	struct common_data *dst_data = pdata.ptr;
	dst_used_offset = dst_data->data_offset;
	ret = rpma_mr_remote_from_descriptor(&dst_data->descriptors[0],
			dst_data->mr_desc_size, &remote_mr);
	if (ret)
		goto err_mr_dereg;

	/* get the remote memory region size */
	if ((ret = rpma_mr_remote_get_size(remote_mr, &remote_size)))
		goto err_mr_remote_delete;

	/* read the used value */
	if ((ret = rpma_read(conn, local_mr, 0,
			remote_mr, dst_used_offset,
			sizeof(uint64_t), RPMA_F_COMPLETION_ALWAYS, NULL)))
		goto err_mr_remote_delete;

	/* get the connection's main CQ */
	struct rpma_cq *cq = NULL;
	if ((ret = rpma_conn_get_cq(conn, &cq)))
		goto err_mr_remote_delete;

	/* wait for the completion to be ready */
	if ((ret = rpma_cq_wait(cq)))
		goto err_mr_remote_delete;

	if ((ret = rpma_cq_get_wc(cq, 1, &wc, NULL)))
		goto err_mr_remote_delete;

	if (wc.status != IBV_WC_SUCCESS) {
		(void) fprintf(stderr, "rpma_read() failed: %s\n",
				ibv_wc_status_str(wc.status));
		ret = -1;
		goto err_mr_remote_delete;
	}

	memcpy(&used, mr_ptr, sizeof(used));
	printf("used value: %lu\n", used.uint64);

	if (remote_size <= used.uint64) {
		fprintf(stderr, "Log size exhausted.\n");
		ret = -1;
		goto err_mr_remote_delete;
	}

	enum rpma_flush_type flush_type;
	int remote_flush_type;
	/*
	 * rpma_mr_remote_get_flush_type() cannot fail here because:
	 * remote_mr != NULL && remote_flush_type != NULL
	 */
	(void) rpma_mr_remote_get_flush_type(remote_mr, &remote_flush_type);
	if (remote_flush_type & RPMA_MR_USAGE_FLUSH_TYPE_PERSISTENT)
		flush_type = RPMA_FLUSH_TYPE_PERSISTENT;
	else
		flush_type = RPMA_FLUSH_TYPE_VISIBILITY;

	for (int i = 3; i < argc; ++i) {
		char *word = mr_ptr;
		strncpy(word, argv[i], MAX_WORD_LENGTH);
		/* make sure the word is always null-terminated */
		word[MAX_WORD_LENGTH] = 0;
		size_t word_size = strlen(word) + 1;

		if ((ret = rpma_write(conn, remote_mr, used.uint64,
				local_mr, 0, word_size,
				RPMA_F_COMPLETION_ON_ERROR, NULL)))
			break;

		if ((ret = rpma_flush(conn, remote_mr, used.uint64,
				word_size, flush_type,
				RPMA_F_COMPLETION_ON_ERROR, NULL)))
			break;

		used.uint64 += word_size;

		if ((ret = rpma_atomic_write(conn, remote_mr, dst_used_offset,
				used.buf, RPMA_F_COMPLETION_ON_ERROR, NULL)))
			break;

		if ((ret = rpma_flush(conn, remote_mr, dst_used_offset,
				sizeof(uint64_t), flush_type,
				RPMA_F_COMPLETION_ALWAYS, FLUSH_ID)))
			break;

		/* wait for the completion to be ready */
		if ((ret = rpma_cq_wait(cq)))
			break;

		if ((ret = rpma_cq_get_wc(cq, 1, &wc, NULL)))
			break;

		if (wc.wr_id != (uintptr_t)FLUSH_ID) {
			(void) fprintf(stderr,
				"unexpected wc.wr_id value "
				"(0x%" PRIXPTR " != 0x%" PRIXPTR ")\n",
				wc.wr_id, (uintptr_t)FLUSH_ID);
			ret = -1;
			break;
		}

		if (wc.status != IBV_WC_SUCCESS) {
			(void) fprintf(stderr, "rpma_flush() failed: %s\n",
					ibv_wc_status_str(wc.status));
			ret = -1;
			break;
		}
	}

err_mr_remote_delete:
	/* delete the remote memory region's structure */
	(void) rpma_mr_remote_delete(&remote_mr);

err_mr_dereg:
	/* deregister the memory region */
	(void) rpma_mr_dereg(&local_mr);

err_peer_cfg_delete:
	(void) rpma_peer_cfg_delete(&pcfg);

err_conn_disconnect:
	(void) common_disconnect_and_wait_for_conn_close(&conn);

err_peer_delete:
	/* delete the peer */
	(void) rpma_peer_delete(&peer);

err_free:
	free(mr_ptr);

	return ret ? -2 : 0;
}
