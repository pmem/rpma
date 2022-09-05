// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020-2022, Intel Corporation */
/* Copyright 2021-2022, Fujitsu */

/*
 * client.c -- a client of the flush-to-persistent-GPSPM example
 *
 * Please see README.md for a detailed description of this example.
 */

#include <librpma.h>
#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>
#include "common-conn.h"
#include "gpspm/flush-to-persistent-GPSPM.h"

/* Generated by the protocol buffer compiler from: GPSPM_flush.proto */
#include "common-hello.h"
#include "common-map_file_with_signature_check.h"
#include "common-pmem_map_file.h"
#include "gpspm/GPSPM_flush.pb-c.h"

#ifdef USE_PMEM
#define USAGE_STR "usage: %s <server_address> <port> [<pmem-path>]\n"PMEM_USAGE
#else
#define USAGE_STR "usage: %s <server_address> <port>\n"
#endif /* USE_PMEM */

#define FLUSH_ID	(void *)0xF01D /* a random identifier */

int
main(int argc, char *argv[])
{
	/* validate parameters */
	if (argc < 3) {
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
	struct common_mem mem;
	memset(&mem, 0, sizeof(mem));
	struct rpma_mr_remote *dst_mr = NULL;
	size_t dst_size = 0;
	size_t dst_offset = 0;
	struct rpma_mr_local *src_mr = NULL;
	struct ibv_wc wc;

	/* messaging resources */
	void *msg_ptr = NULL;
	void *send_ptr = NULL;
	void *recv_ptr = NULL;
	struct rpma_mr_local *msg_mr = NULL;
	GPSPMFlushRequest flush_req = GPSPM_FLUSH_REQUEST__INIT;
	size_t flush_req_size = 0;
	GPSPMFlushResponse *flush_resp = NULL;

	struct hello_t *hello = NULL;

#ifdef USE_PMEM
	if (argc >= 4) {
		char *path = argv[3];

		ret = common_pmem_map_file_with_signature_check(path, 0, &mem);
		if (ret)
			goto err_free;

		hello = (struct hello_t *)((uintptr_t)mem.mr_ptr + mem.data_offset);
	}
#endif /* USE_PMEM */

	/* if no pmem support or it is not provided */
	if (mem.mr_ptr == NULL) {
		(void) fprintf(stderr, NO_PMEM_MSG);
		mem.mr_ptr = malloc_aligned(sizeof(struct hello_t));
		if (mem.mr_ptr == NULL)
			return -1;

		mem.mr_size = sizeof(struct hello_t);
		hello = (struct hello_t *)mem.mr_ptr;

		/* write an initial value */
		write_hello_str(hello, en);
	}

	(void) printf("Next value: %s\n", hello->str);

	/* allocate messaging buffer */
	msg_ptr = malloc_aligned(KILOBYTE);
	if (msg_ptr == NULL) {
		ret = -1;
		goto err_free;
	}
	send_ptr = (char *)msg_ptr + SEND_OFFSET;
	recv_ptr = (char *)msg_ptr + RECV_OFFSET;

	/* RPMA resources */
	struct rpma_peer *peer = NULL;
	struct rpma_conn *conn = NULL;

	/*
	 * lookup an ibv_context via the address and create a new peer using it
	 */
	if ((ret = client_peer_via_address(addr, &peer)))
		goto err_free;

	struct rpma_conn_cfg *cfg = NULL;
	if ((ret = rpma_conn_cfg_new(&cfg)))
		goto err_peer_delete;

	if ((ret = rpma_conn_cfg_set_rcq_size(cfg, RCQ_SIZE)))
		goto err_cfg_delete;

	if ((ret = rpma_conn_cfg_set_compl_channel(cfg, true)))
		goto err_cfg_delete;

	/* establish a new connection to a server listening at addr:port */
	if ((ret = client_connect(peer, addr, port, cfg, NULL, &conn)))
		goto err_cfg_delete;

	/* register the memory RDMA write */
	if ((ret = rpma_mr_reg(peer, mem.mr_ptr, mem.mr_size, RPMA_MR_USAGE_WRITE_SRC, &src_mr)))
		goto err_conn_disconnect;

	/* register the messaging memory */
	if ((ret = rpma_mr_reg(peer, msg_ptr, KILOBYTE, RPMA_MR_USAGE_SEND | RPMA_MR_USAGE_RECV,
			&msg_mr))) {
		(void) rpma_mr_dereg(&src_mr);
		goto err_conn_disconnect;
	}

	/* obtain the remote side resources description */
	struct rpma_conn_private_data pdata;
	ret = rpma_conn_get_private_data(conn, &pdata);
	if (ret != 0 || pdata.len < sizeof(struct common_data))
		goto err_mr_dereg;

	/*
	 * Create a remote memory registration structure from the received descriptor.
	 */
	struct common_data *dst_data = pdata.ptr;

	if ((ret = rpma_mr_remote_from_descriptor(&dst_data->descriptors[0],
			dst_data->mr_desc_size, &dst_mr)))
		goto err_mr_dereg;

	/* get the remote memory region size */
	if ((ret = rpma_mr_remote_get_size(dst_mr, &dst_size))) {
		goto err_mr_remote_delete;
	} else if (dst_size < KILOBYTE) {
		fprintf(stderr,
				"Size of the remote memory region is too small for writing the data of the assumed size (%zu < %d)\n",
				dst_size, KILOBYTE);
		goto err_mr_remote_delete;
	}

	dst_offset = dst_data->data_offset;
	if ((ret = rpma_write(conn, dst_mr, dst_offset, src_mr,
			(mem.data_offset + offsetof(struct hello_t, str)), KILOBYTE,
			RPMA_F_COMPLETION_ON_ERROR, NULL)))
		goto err_mr_remote_delete;

	/* prepare a response buffer */
	if ((ret = rpma_recv(conn, msg_mr, RECV_OFFSET, MSG_SIZE_MAX, NULL)))
		goto err_mr_remote_delete;

	/* prepare a flush message and pack it to a send buffer */
	flush_req.offset = dst_offset;
	flush_req.length = KILOBYTE;
	flush_req.op_context = (uint64_t)FLUSH_ID;
	flush_req_size = gpspm_flush_request__get_packed_size(&flush_req);
	if (flush_req_size > MSG_SIZE_MAX) {
		fprintf(stderr,
				"Packed flush request size is bigger than available send buffer space (%"
				PRIu64 " > %d\n", flush_req_size, MSG_SIZE_MAX);
		goto err_mr_remote_delete;
	}
	(void) gpspm_flush_request__pack(&flush_req, send_ptr);

	/* send the flush message */
	if ((ret = rpma_send(conn, msg_mr, SEND_OFFSET, flush_req_size, RPMA_F_COMPLETION_ALWAYS,
			NULL)))
		goto err_mr_remote_delete;

	/* wait for the send completion to be ready */
	if ((ret = wait_and_validate_completion(conn, IBV_WC_SEND, &wc)))
		goto err_mr_remote_delete;

	/* wait for the receive completion to be ready */
	if ((ret = wait_and_validate_completion(conn, IBV_WC_RECV, &wc)))
		goto err_mr_remote_delete;

	/* unpack a response from the received buffer */
	flush_resp = gpspm_flush_response__unpack(NULL, wc.byte_len, recv_ptr);
	if (flush_resp == NULL) {
		fprintf(stderr, "Cannot unpack the flush response buffer\n");
		goto err_mr_remote_delete;
	}
	if (flush_resp->op_context != (uint64_t)FLUSH_ID) {
		(void) fprintf(stderr, "unexpected flush_resp->op_context value "
				"(0x%" PRIXPTR " != 0x%" PRIXPTR ")\n",
				(uintptr_t)flush_resp->op_context, (uintptr_t)FLUSH_ID);
		goto err_mr_remote_delete;
	}
	gpspm_flush_response__free_unpacked(flush_resp, NULL);

	/*
	 * Translate the message so the next time the greeting will be surprising.
	 */
	translate(hello);
#ifdef USE_PMEM
	if (mem.is_pmem) {
		mem.persist(hello, sizeof(struct hello_t));
	} else
#endif /* USE_PMEM */

	(void) printf("Translation: %s\n", hello->str);

err_mr_remote_delete:
	/* delete the remote memory region's structure */
	(void) rpma_mr_remote_delete(&dst_mr);

err_mr_dereg:
	(void) rpma_mr_dereg(&msg_mr);
	(void) rpma_mr_dereg(&src_mr);

err_conn_disconnect:
	(void) common_disconnect_and_wait_for_conn_close(&conn);

err_cfg_delete:
	(void) rpma_conn_cfg_delete(&cfg);

err_peer_delete:
	/* delete the peer */
	(void) rpma_peer_delete(&peer);

err_free:
	free(msg_ptr);

#ifdef USE_PMEM
	if (mem.is_pmem) {
		common_pmem_unmap_file(&mem);
	} else
#endif /* USE_PMEM */

	if (mem.mr_ptr != NULL)
		free(mem.mr_ptr);

	return ret;
}
