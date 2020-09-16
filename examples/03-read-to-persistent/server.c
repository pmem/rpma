// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020, Intel Corporation */

/*
 * server.c -- a server of the read-to-persistent example
 *
 * Please see README.md for a detailed description of this example.
 */

#include <inttypes.h>
#include <librpma.h>
#include <librpma_log.h>
#include <stdlib.h>
#include <stdio.h>

#ifdef USE_LIBPMEM
#include <libpmem.h>
#define USAGE_STR "usage: %s <server_address> <port> [<pmem-path>]\n"
#else
#define USAGE_STR "usage: %s <server_address> <port>\n"
#endif /* USE_LIBPMEM */

#include "common-conn.h"

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
	void *dst_ptr = NULL;
	size_t dst_size = 0;
	size_t dst_offset = 0;
	enum rpma_mr_plt dst_plt = RPMA_MR_PLT_VOLATILE;
	struct rpma_mr_local *dst_mr = NULL;
	struct rpma_mr_remote *src_mr = NULL;

#ifdef USE_LIBPMEM
	if (argc >= 4) {
		char *path = argv[3];
		int is_pmem;

		/* map the file */
		dst_ptr = pmem_map_file(path, 0 /* len */, 0 /* flags */,
				0 /* mode */, &dst_size, &is_pmem);
		if (dst_ptr == NULL)
			return -1;

		/* pmem is expected */
		if (!is_pmem) {
			(void) pmem_unmap(dst_ptr, dst_size);
			return -1;
		}

		/*
		 * At the beginning of the persistent memory, a signature is
		 * stored which marks its content as valid. So the length
		 * of the mapped memory has to be at least of the length of
		 * the signature to convey any meaningful content and be usable
		 * as a persistent store.
		 */
		if (dst_size < SIGNATURE_LEN) {
			(void) fprintf(stderr, "%s too small (%zu < %zu)\n",
					path, dst_size, SIGNATURE_LEN);
			(void) pmem_unmap(dst_ptr, dst_size);
			return -1;
		}
		dst_offset = SIGNATURE_LEN;

		/*
		 * All of the space under the offset is intended for
		 * the string contents. Space is assumed to be at least 1 KiB.
		 */
		if (dst_size - dst_offset < KILOBYTE) {
			fprintf(stderr, "%s too small (%zu < %zu)\n",
					path, dst_size, KILOBYTE + dst_offset);
			(void) pmem_unmap(dst_ptr, dst_size);
			return -1;
		}

		/*
		 * If the signature is not in place the persistent content has
		 * to be initialized and persisted.
		 */
		if (strncmp(dst_ptr, SIGNATURE_STR, SIGNATURE_LEN) != 0) {
			/* write an initial empty string and persist it */
			((char *)dst_ptr + dst_offset)[0] = '\0';
			pmem_persist(dst_ptr, 1);
			/* write the signature to mark the content as valid */
			memcpy(dst_ptr, SIGNATURE_STR, SIGNATURE_LEN);
			pmem_persist(dst_ptr, SIGNATURE_LEN);
		}

		dst_plt = RPMA_MR_PLT_PERSISTENT;
	}
#endif

	/* if no pmem support or it is not provided */
	if (dst_ptr == NULL) {
		dst_ptr = malloc_aligned(KILOBYTE);
		if (dst_ptr == NULL)
			return -1;

		dst_size = KILOBYTE;
		dst_plt = RPMA_MR_PLT_VOLATILE;
	}

	/* RPMA resources */
	struct rpma_peer *peer = NULL;
	struct rpma_ep *ep = NULL;
	struct rpma_conn *conn = NULL;
	struct rpma_completion cmpl;

	/*
	 * lookup an ibv_context via the address and create a new peer using it
	 */
	ret = server_peer_via_address(addr, &peer);
	if (ret)
		goto err_free;

	/* start a listening endpoint at addr:port */
	ret = rpma_ep_listen(peer, addr, port, &ep);
	if (ret)
		goto err_peer_delete;

	/* register the memory */
	ret = rpma_mr_reg(peer, dst_ptr, dst_size, RPMA_MR_USAGE_READ_DST,
			dst_plt, &dst_mr);
	if (ret)
		goto err_ep_shutdown;

	/*
	 * Wait for an incoming connection request, accept it and wait for its
	 * establishment.
	 */
	ret = server_accept_connection(ep, NULL, &conn);
	if (ret)
		goto err_mr_dereg;

	/* obtain the remote memory description */
	struct rpma_conn_private_data pdata;
	ret = rpma_conn_get_private_data(conn, &pdata);
	if (ret != 0 || pdata.len < sizeof(struct common_data))
		goto err_disconnect;

	struct common_data *src_data = pdata.ptr;
	ret = rpma_mr_remote_from_descriptor(
			&src_data->descriptors[src_data->mr_desc_offset],
			src_data->mr_desc_size,
			&src_mr);
	if (ret)
		goto err_disconnect;

	/* if the string content is not empty */
	if (((char *)dst_ptr + dst_offset)[0] != '\0') {
		(void) printf("Old value: %s\n", (char *)dst_ptr + dst_offset);
	}

	ret = rpma_read(conn, dst_mr, dst_offset, src_mr, src_data->data_offset,
			KILOBYTE, RPMA_F_COMPLETION_ALWAYS, NULL);
	if (ret)
		goto err_mr_remote_delete;

	/* wait for the completion to be ready */
	ret = rpma_conn_prepare_completions(conn);
	if (ret)
		goto err_mr_remote_delete;

	ret = rpma_conn_next_completion(conn, &cmpl);
	if (ret)
		goto err_mr_remote_delete;

	if (cmpl.op != RPMA_OP_READ) {
		(void) fprintf(stderr,
				"unexpected cmpl.op value (%d != %d)\n",
				cmpl.op, RPMA_OP_READ);
		goto err_mr_remote_delete;
	}

	if (cmpl.op_status != IBV_WC_SUCCESS) {
		(void) fprintf(stderr, "rpma_read failed with %d\n",
				cmpl.op_status);
		goto err_mr_remote_delete;
	}

#ifdef USE_LIBPMEM
	if (dst_plt == RPMA_MR_PLT_PERSISTENT) {
		pmem_persist((char *)dst_ptr + dst_offset, KILOBYTE);
	}
#endif

	(void) printf("New value: %s\n", (char *)dst_ptr + dst_offset);

err_mr_remote_delete:
	(void) rpma_mr_remote_delete(&src_mr);

err_disconnect:
	/*
	 * Wait for RPMA_CONN_CLOSED, disconnect and delete the connection
	 * structure.
	 */
	ret = common_disconnect_and_wait_for_conn_close(&conn);

err_mr_dereg:
	/* deregister the memory region */
	(void) rpma_mr_dereg(&dst_mr);

err_ep_shutdown:
	/* shutdown the endpoint */
	(void) rpma_ep_shutdown(&ep);

err_peer_delete:
	/* delete the peer object */
	(void) rpma_peer_delete(&peer);

err_free:
#ifdef USE_LIBPMEM
	if (dst_plt == RPMA_MR_PLT_PERSISTENT) {
		pmem_unmap(dst_ptr, dst_size);
		dst_ptr = NULL;
	}
#endif

	if (dst_ptr != NULL)
		free(dst_ptr);

	return ret;
}
