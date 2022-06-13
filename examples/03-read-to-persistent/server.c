// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020-2022, Intel Corporation */
/* Copyright 2021-2022, Fujitsu */

/*
 * server.c -- a server of the read-to-persistent example
 *
 * Please see README.md for a detailed description of this example.
 */

#include <inttypes.h>
#include <librpma.h>
#include <stdlib.h>
#include <stdio.h>
#include "common-conn.h"
#include "common-pmem_map_file.h"

#ifdef USE_PMEM
#define USAGE_STR "usage: %s <server_address> <port> [<pmem-path>]\n"PMEM_USAGE
#else
#define USAGE_STR "usage: %s <server_address> <port>\n"
#endif /* USE_PMEM */

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
	struct example_mem mem;
	memset(&mem, 0, sizeof(mem));
	struct rpma_mr_local *dst_mr = NULL;
	struct rpma_mr_remote *src_mr = NULL;

#ifdef USE_PMEM
	char *pmem_path = NULL;
	if (argc >= 4) {
		pmem_path = argv[3];

		if (client_pmem_map_file(pmem_path, &mem, SIGNATURE_LEN + KILOBYTE))
			return -1;

		/*
		 * At the beginning of the persistent memory, a signature is
		 * stored which marks its content as valid. So the length
		 * of the mapped memory has to be at least of the length of
		 * the signature to convey any meaningful content and be usable
		 * as a persistent store.
		 */
		if (mem.mr_size < SIGNATURE_LEN) {
			(void) fprintf(stderr, "%s too small (%zu < %zu)\n",
					pmem_path, mem.mr_size, SIGNATURE_LEN);
			ret = -1;
			goto err_free;
		}
		mem.data_offset = SIGNATURE_LEN;

		/*
		 * All of the space under the offset is intended for
		 * the string contents. Space is assumed to be at least 1 KiB.
		 */
		if (mem.mr_size - mem.data_offset < KILOBYTE) {
			fprintf(stderr, "%s too small (%zu < %zu)\n",
				pmem_path, mem.mr_size, KILOBYTE + mem.data_offset);
			ret = -1;
			goto err_free;
		}

		/*
		 * If the signature is not in place the persistent content has
		 * to be initialized and persisted.
		 */
		if (strncmp(mem.mr_ptr, SIGNATURE_STR, SIGNATURE_LEN) != 0) {
			/* write an initial empty string and persist it */
			((char *)mem.mr_ptr + mem.data_offset)[0] = '\0';
			mem.persist(mem.mr_ptr, 1);
			/* write the signature to mark the content as valid */
			memcpy(mem.mr_ptr, SIGNATURE_STR, SIGNATURE_LEN);
			mem.persist(mem.mr_ptr, SIGNATURE_LEN);
		}
	}
#endif /* USE_PMEM */

	/* if no pmem support or it is not provided */
	if (mem.mr_ptr == NULL) {
		(void) fprintf(stderr, NO_PMEM_MSG);
		mem.mr_ptr = malloc_aligned(KILOBYTE);
		if (mem.mr_ptr == NULL)
			return -1;

		mem.mr_size = KILOBYTE;
	}

	/* RPMA resources */
	struct rpma_peer *peer = NULL;
	struct rpma_ep *ep = NULL;
	struct rpma_conn *conn = NULL;
	struct ibv_wc wc;

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
	ret = rpma_mr_reg(peer, mem.mr_ptr, mem.mr_size, RPMA_MR_USAGE_READ_DST,
				&dst_mr);
	if (ret)
		goto err_ep_shutdown;

#ifdef USE_PMEM
	/* rpma_mr_advise() should be called only in case of FsDAX */
	if (mem.is_pmem && strstr(pmem_path, "/dev/dax") == NULL) {
		ret = rpma_mr_advise(dst_mr, 0, mem.mr_size,
			IBV_ADVISE_MR_ADVICE_PREFETCH_WRITE,
			IBV_ADVISE_MR_FLAG_FLUSH);
		if (ret)
			goto err_mr_dereg;
	}
#endif /* USE_PMEM */

	/*
	 * Wait for an incoming connection request, accept it and wait for its
	 * establishment.
	 */
	ret = server_accept_connection(ep, NULL, NULL, &conn);
	if (ret)
		goto err_mr_dereg;

	/* obtain the remote memory description */
	struct rpma_conn_private_data pdata;
	ret = rpma_conn_get_private_data(conn, &pdata);
	if (ret != 0 || pdata.len < sizeof(struct common_data))
		goto err_disconnect;

	struct common_data *src_data = pdata.ptr;

	ret = rpma_mr_remote_from_descriptor(&src_data->descriptors[0],
			src_data->mr_desc_size, &src_mr);
	if (ret)
		goto err_disconnect;

	/* if the string content is not empty */
	if (((char *)mem.mr_ptr + mem.data_offset)[0] != '\0') {
		(void) printf("Old value: %s\n", (char *)mem.mr_ptr + mem.data_offset);
	}

	ret = rpma_read(conn, dst_mr, mem.data_offset, src_mr, src_data->data_offset,
			KILOBYTE, RPMA_F_COMPLETION_ALWAYS, NULL);
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

#ifdef USE_PMEM
	if (mem.is_pmem) {
		mem.persist((char *)mem.mr_ptr + mem.data_offset, KILOBYTE);
	}
#endif /* USE_PMEM */

	(void) printf("New value: %s\n", (char *)mem.mr_ptr + mem.data_offset);

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
#ifdef USE_PMEM
	if (mem.is_pmem) {
		client_pmem_unmap_file(&mem);
	}
#endif

	if (mem.mr_ptr != NULL)
		free(mem.mr_ptr);

	return ret;
}
