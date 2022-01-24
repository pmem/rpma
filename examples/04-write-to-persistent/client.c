// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020-2022, Intel Corporation */
/* Copyright 2021, Fujitsu */

/*
 * client.c -- a client of the write-to-persistent example
 *
 * Please see README.md for a detailed description of this example.
 */

#include <librpma.h>
#include <stdlib.h>
#include <stdio.h>
#include "common-conn.h"
#include "hello.h"

#ifdef USE_LIBPMEM
#include <libpmem.h>
#define USAGE_STR "usage: %s <server_address> <port> [<pmem-path>]\n"PMEM_USAGE
#else
#define USAGE_STR "usage: %s <server_address> <port>\n"
#endif /* USE_LIBPMEM */

static inline void
write_hello_str(struct hello_t *hello, enum lang_t lang)
{
	hello->lang = lang;
	strncpy(hello->str, hello_str[hello->lang], KILOBYTE - 1);
	hello->str[KILOBYTE - 1] = '\0';
}

static void
translate(struct hello_t *hello)
{
	printf("translating...\n");
	enum lang_t lang = (enum lang_t)((hello->lang + 1) % LANG_NUM);
	write_hello_str(hello, lang);
}

#ifdef TEST_USE_CMOCKA
#include "cmocka_headers.h"
#include "cmocka_alloc.h"
#endif

#ifdef TEST_MOCK_MAIN
#define main client_main
#endif

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
	void *mr_ptr = NULL;
	size_t mr_size = 0;
	size_t data_offset = 0;
	struct rpma_mr_remote *dst_mr = NULL;
	size_t dst_size = 0;
	size_t dst_offset = 0;
	struct rpma_mr_local *src_mr = NULL;
	struct rpma_completion cmpl;

	/* read-after-write memory region */
	void *raw = NULL;
	struct rpma_mr_local *raw_mr = NULL;

	struct hello_t *hello = NULL;

#ifdef USE_LIBPMEM
	int is_pmem = 0;
	if (argc >= 4) {
		char *path = argv[3];

		/* map the file */
		mr_ptr = pmem_map_file(path, 0 /* len */, 0 /* flags */,
				0 /* mode */, &mr_size, &is_pmem);
		if (mr_ptr == NULL) {
			(void) fprintf(stderr, "pmem_map_file() for %s "
					"failed\n", path);
			return -1;
		}

		/* pmem is expected */
		if (!is_pmem) {
			(void) fprintf(stderr, "%s is not an actual PMEM\n",
					path);
			(void) pmem_unmap(mr_ptr, mr_size);
			return -1;
		}

		/*
		 * At the beginning of the persistent memory, a signature is
		 * stored which marks its content as valid. So the length
		 * of the mapped memory has to be at least of the length of
		 * the signature to convey any meaningful content and be usable
		 * as a persistent store.
		 */
		if (mr_size < SIGNATURE_LEN) {
			(void) fprintf(stderr, "%s too small (%zu < %zu)\n",
					path, mr_size, SIGNATURE_LEN);
			(void) pmem_unmap(mr_ptr, mr_size);
			return -1;
		}
		data_offset = SIGNATURE_LEN;

		/*
		 * The space under the offset is intended for storing the hello
		 * structure. So the total space is assumed to be at least
		 * 1 KiB + offset of the string contents.
		 */
		if (mr_size - data_offset < sizeof(struct hello_t)) {
			fprintf(stderr, "%s too small (%zu < %zu)\n",
					path, mr_size, sizeof(struct hello_t));
			(void) pmem_unmap(mr_ptr, mr_size);
			return -1;
		}

		hello = (struct hello_t *)((uintptr_t)mr_ptr + data_offset);

		/*
		 * If the signature is not in place the persistent content has
		 * to be initialized and persisted.
		 */
		if (strncmp(mr_ptr, SIGNATURE_STR, SIGNATURE_LEN) != 0) {
			/* write an initial value and persist it */
			write_hello_str(hello, en);
			pmem_persist(hello, sizeof(struct hello_t));
			/* write the signature to mark the content as valid */
			memcpy(mr_ptr, SIGNATURE_STR, SIGNATURE_LEN);
			pmem_persist(mr_ptr, SIGNATURE_LEN);
		}
	}
#endif

	/* if no pmem support or it is not provided */
	if (mr_ptr == NULL) {
		(void) fprintf(stderr, NO_PMEM_MSG);
		mr_ptr = malloc_aligned(sizeof(struct hello_t));
		if (mr_ptr == NULL)
			return -1;

		mr_size = sizeof(struct hello_t);
		hello = mr_ptr;

		/* write an initial value */
		write_hello_str(hello, en);
	}

	/* alloc memory for the read-after-write buffer (RAW) */
	raw = malloc_aligned(KILOBYTE);
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
	ret = rpma_mr_reg(peer, mr_ptr, mr_size, RPMA_MR_USAGE_WRITE_SRC,
				&src_mr);
	if (ret)
		goto err_conn_disconnect;

	/* register the RAW buffer */
	ret = rpma_mr_reg(peer, raw, 8, RPMA_MR_USAGE_READ_DST, &raw_mr);
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

	/* get the remote memory region size */
	ret = rpma_mr_remote_get_size(dst_mr, &dst_size);
	if (ret) {
		goto err_mr_dereg;
	} else if (dst_size < KILOBYTE) {
		ret = -1;
		fprintf(stderr,
				"Remote memory region size too small for writing the data of the assumed size (%zu < %d)\n",
				dst_size, KILOBYTE);
		goto err_mr_remote_delete;
	}

	dst_offset = dst_data->data_offset;
	ret = rpma_write(conn, dst_mr, dst_offset, src_mr,
			(data_offset + offsetof(struct hello_t, str)), KILOBYTE,
			RPMA_F_COMPLETION_ON_ERROR, NULL);
	if (ret)
		goto err_mr_remote_delete;

	/* the read serves here as flushing primitive */
	ret = rpma_read(conn, raw_mr, 0, dst_mr, 0, 8,
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

	ret = rpma_cq_get_completion(cq, &cmpl);
	if (ret)
		goto err_mr_remote_delete;

	if (cmpl.op_status != IBV_WC_SUCCESS) {
		ret = -1;
		(void) fprintf(stderr, "rpma_read() failed: %s\n",
				ibv_wc_status_str(cmpl.op_status));
		goto err_mr_remote_delete;
	}

	if (cmpl.op != RPMA_OP_READ) {
		ret = -1;
		(void) fprintf(stderr,
				"unexpected cmpl.op value (%d != %d)\n",
				cmpl.op, RPMA_OP_READ);
		goto err_mr_remote_delete;
	}

	/*
	 * Translate the message so the next time the greeting will be
	 * surprising.
	 */
	translate(hello);
#ifdef USE_LIBPMEM
	if (is_pmem) {
		pmem_persist(hello, sizeof(struct hello_t));
	}
#endif

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
#ifdef USE_LIBPMEM
	if (is_pmem) {
		pmem_unmap(mr_ptr, mr_size);
		mr_ptr = NULL;
	}
#endif

	if (mr_ptr != NULL)
		free(mr_ptr);

	if (raw != NULL)
		free(raw);

	return ret;
}
