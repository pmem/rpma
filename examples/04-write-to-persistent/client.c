// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020-2022, Intel Corporation */
/* Copyright 2021-2022, Fujitsu */

/*
 * client.c -- a client of the write-to-persistent example
 *
 * Please see README.md for a detailed description of this example.
 */

#include <librpma.h>
#include <stdlib.h>
#include <stdio.h>
#include "common-conn.h"
#include "common-hello.h"
#include "common-map_file_with_signature_check.h"
#include "common-pmem_map_file.h"

#ifdef USE_PMEM
#define USAGE_STR "usage: %s <server_address> <port> [<pmem-path>]\n"PMEM_USAGE
#else
#define USAGE_STR "usage: %s <server_address> <port>\n"
#endif /* USE_PMEM */

#ifdef TEST_USE_CMOCKA
#include "cmocka_headers.h"
#include "cmocka_alloc.h"
#endif

#ifdef TEST_MOCK_MAIN
#define main client_main
#endif

/* read-after-write buffer size */
#define RAW_BUFFER_SIZE 8

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

	/* resources - memory region */
	struct common_mem mem;
	memset(&mem, 0, sizeof(mem));
	struct rpma_mr_remote *dst_mr = NULL;
	size_t dst_size = 0;
	size_t dst_offset = 0;
	struct rpma_mr_local *src_mr = NULL;
	struct ibv_wc wc;

	/* read-after-write memory region */
	void *raw = NULL;
	struct rpma_mr_local *raw_mr = NULL;
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

	/* alloc memory for the read-after-write buffer (RAW) */
	raw = malloc_aligned(RAW_BUFFER_SIZE);
	if (raw == NULL) {
		ret = -1;
		goto err_free;
	}

	(void) printf("Next value: %s\n", hello->str);

	/* RPMA resources */
	struct rpma_peer *peer = NULL;
	struct rpma_conn *conn = NULL;

	/*
	 * lookup an ibv_context via the address and create a new peer using it
	 */
	ret = client_peer_via_address(addr, &peer);
	if (ret)
		goto err_free;

	/* establish a new connection to a server listening at addr:port */
	ret = client_connect(peer, addr, port, NULL, NULL, &conn);
	if (ret)
		goto err_peer_delete;

	/* register the memory RDMA write */
	ret = rpma_mr_reg(peer, mem.mr_ptr, mem.mr_size, RPMA_MR_USAGE_WRITE_SRC,
				&src_mr);
	if (ret)
		goto err_conn_disconnect;

	/* register the RAW buffer */
	ret = rpma_mr_reg(peer, raw, RAW_BUFFER_SIZE, RPMA_MR_USAGE_READ_DST, &raw_mr);
	if (ret)
		goto err_mr_dereg;

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

	ret = rpma_mr_remote_from_descriptor(&dst_data->descriptors[0],
			dst_data->mr_desc_size, &dst_mr);
	if (ret)
		goto err_mr_dereg;

	dst_offset = dst_data->data_offset;

	/* get the remote memory region size */
	ret = rpma_mr_remote_get_size(dst_mr, &dst_size);
	if (ret) {
		goto err_mr_remote_delete;
	} else if (dst_size - dst_offset < HELLO_STR_SIZE) {
		ret = -1;
		fprintf(stderr,
				"Remote memory region size too small for writing the data of the assumed size (%zu < %d)\n",
				dst_size - dst_offset, HELLO_STR_SIZE);
		goto err_mr_remote_delete;
	}

	ret = rpma_write(conn, dst_mr, dst_offset, src_mr,
			(mem.data_offset + offsetof(struct hello_t, str)), HELLO_STR_SIZE,
			RPMA_F_COMPLETION_ON_ERROR, NULL);
	if (ret)
		goto err_mr_remote_delete;

	/* the read serves here as flushing primitive */
	ret = rpma_read(conn, raw_mr, 0, dst_mr, 0, RAW_BUFFER_SIZE,
			RPMA_F_COMPLETION_ALWAYS, NULL);
	if (ret)
		goto err_mr_remote_delete;

	/* get the connection's main CQ */
	struct rpma_cq *cq = NULL;
	ret = rpma_conn_get_cq(conn, &cq);
	if (ret)
		goto err_mr_remote_delete;

	/* wait for the completion to be ready */
	ret = rpma_cq_wait(cq);
	if (ret)
		goto err_mr_remote_delete;

	ret = rpma_cq_get_wc(cq, 1, &wc, NULL);
	if (ret)
		goto err_mr_remote_delete;

	if (wc.status != IBV_WC_SUCCESS) {
		ret = -1;
		(void) fprintf(stderr, "rpma_read() failed: %s\n",
				ibv_wc_status_str(wc.status));
		goto err_mr_remote_delete;
	}

	if (wc.opcode != IBV_WC_RDMA_READ) {
		ret = -1;
		(void) fprintf(stderr,
				"unexpected wc.opcode value (%d != %d)\n",
				wc.opcode, IBV_WC_RDMA_READ);
		goto err_mr_remote_delete;
	}

	/*
	 * Translate the message so the next time the greeting will be
	 * surprising.
	 */
	translate(hello);
#ifdef USE_PMEM
	if (mem.is_pmem) {
		mem.persist(hello, sizeof(struct hello_t));
	}
#endif /* USE_PMEM */

	(void) printf("Translation: %s\n", hello->str);

err_mr_remote_delete:
	/* delete the remote memory region's structure */
	(void) rpma_mr_remote_delete(&dst_mr);

err_mr_dereg:
	/* deregister the memory region */
	(void) rpma_mr_dereg(&src_mr);
	(void) rpma_mr_dereg(&raw_mr);

err_conn_disconnect:
	(void) common_disconnect_and_wait_for_conn_close(&conn);

err_peer_delete:
	/* delete the peer */
	(void) rpma_peer_delete(&peer);

err_free:
#ifdef USE_PMEM
	if (mem.is_pmem) {
		common_pmem_unmap_file(&mem);
	} else
#endif /* USE_PMEM */
	if (mem.mr_ptr != NULL)
		free(mem.mr_ptr);

	if (raw != NULL)
		free(raw);

	return ret;
}
