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

#ifdef USE_LIBPMEM2
#include <libpmem2.h>
#define USAGE_STR "usage: %s <server_address> <port> [<pmem-path>]\n"
#else
#define USAGE_STR "usage: %s <server_address> <port>\n"
#endif /* USE_LIBPMEM2 */

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
	size_t dst_offset = 0;
	struct rpma_mr_local *dst_mr = NULL;
	struct rpma_mr_remote *src_mr = NULL;

#ifdef USE_LIBPMEM2

	struct pmem2_config *cfg = NULL;
	struct pmem2_map *map = NULL;
	struct pmem2_source *src = NULL;
	struct pmem2_persist_fn persist;
	struct pmem2_memcpy_fn copy;
	int fd;

	if (argc >= 4)
	{
		char *path = argv[3];

		if ((fd = open(argv[1], O_RDWR)) < 0) {
		perror("open");
		exit(1);
	}


		if (pmem2_config_new(&cfg) != 0) {
			pmem2_perror("pmem2_config_new");
		goto err_source_delete;
	}

		if (pmem2_source_from_fd(&src, fd) != 0) {
			pmem2_perror("pmem2_source_from_fd");
		goto err_close;
	}

		if (pmem2_config_set_required_store_granularity(cfg,
			PMEM2_GRANULARITY_PAGE)) {
		pmem2_perror("pmem2_config_set_required_store_granularity");
		goto err_config_delete;
	}

		int err = pmem2_map_new(&map, cfg, src);


		/* pmem is expected */
		if (!err == PMEM2_E_GRANULARITY_NOT_SUPPORTED)
		{
			(void)fprintf(stderr, "%s is not an actual PMEM\n",
				      path);
			return -1;
		}

		else if (err)
		{
			pmem2_perror("pmem2_map_new");
			return -1;
		}
 		
		persist = pmem2_get_persist_fn(map);
		copy = pmem2_get_memcpy_fn(map);
		mr_ptr = pmem2_map_get_address(map);
		mr_size = pmem2_map_get_size(map);
		

		/*
		 * At the beginning of the persistent memory, a signature is
		 * stored which marks its content as valid. So the length
		 * of the mapped memory has to be at least of the length of
		 * the signature to convey any meaningful content and be usable
		 * as a persistent store.
		 */
		if (mr_size < SIGNATURE_LEN)
		{
			(void)fprintf(stderr, "%s too small (%zu < %zu)\n",
				      path, mr_size, SIGNATURE_LEN);
			(void)pmem2_unmap(mr_ptr, mr_size);
			return -1;
		}
		data_offset = SIGNATURE_LEN;

		/*
		 * The space under the offset is intended for storing the hello
		 * structure. So the total space is assumed to be at least
		 * 1 KiB + offset of the string contents.
		 */
		if (mr_size - data_offset < sizeof(struct hello_t))
		{
			fprintf(stderr, "%s too small (%zu < %zu)\n",
				path, mr_size, sizeof(struct hello_t));
			return -1;
		}

		hello = (struct hello_t *)((uintptr_t)mr_ptr + data_offset);

		/*
		 * If the signature is not in place the persistent content has
		 * to be initialized and persisted.
		 */
		if (strncmp(mr_ptr, SIGNATURE_STR, SIGNATURE_LEN) != 0)
		{
			/* write an initial value and persist it */
			write_hello_str(hello, en);
			persist(hello, sizeof(struct hello_t));
			/* write the signature to mark the content as valid */
			copy(mr_ptr, SIGNATURE_STR, SIGNATURE_LEN, 0);
			
		}
	}
#endif

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

	/* if no pmem2 support or it is not provided */
	if (mr_ptr == NULL) {
		(void) fprintf(stderr, NO_PMEM_MSG);
		mr_ptr = malloc_aligned(KILOBYTE);
		if (mr_ptr == NULL)
			return -1;

		mr_size = KILOBYTE;
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
	ret = rpma_mr_reg(peer, mr_ptr, mr_size, RPMA_MR_USAGE_READ_DST,
				&dst_mr);
	if (ret)
		goto err_ep_shutdown;

#ifdef USE_LIBPMEM2
	/* rpma_mr_advise() should be called only in case of FsDAX */
	if (err && strstr(pmem_path, "/dev/dax") == NULL) {
		ret = rpma_mr_advise(dst_mr, 0, mr_size,
			IBV_ADVISE_MR_ADVICE_PREFETCH_WRITE,
			IBV_ADVISE_MR_FLAG_FLUSH);
		if (ret)
			goto err_mr_dereg;
	}
#endif /* USE_LIBPMEM2 */

#ifdef USE_LIBPMEM
	/* rpma_mr_advise() should be called only in case of FsDAX */
	if (is_pmem && strstr(pmem_path, "/dev/dax") == NULL) {
		ret = rpma_mr_advise(dst_mr, 0, mr_size,
			IBV_ADVISE_MR_ADVICE_PREFETCH_WRITE,
			IBV_ADVISE_MR_FLAG_FLUSH);
		if (ret)
			goto err_mr_dereg;
	}
#endif /* USE_LIBPMEM */

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
	if (((char *)mr_ptr + dst_offset)[0] != '\0') {
		(void) printf("Old value: %s\n", (char *)mr_ptr + dst_offset);
	}

	ret = rpma_read(conn, dst_mr, dst_offset, src_mr, src_data->data_offset,
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

#ifdef USE_LIBPMEM2
	if (err) {
		persist((char *)mr_ptr + dst_offset, KILOBYTE);
	}
#endif /* USE_LIBPMEM2 */

#ifdef USE_LIBPMEM
	if (is_pmem) {
		pmem_persist((char *)mr_ptr + dst_offset, KILOBYTE);
	}
#endif /* USE_LIBPMEM */

	(void) printf("New value: %s\n", (char *)mr_ptr + dst_offset);

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

err_source_delete:
	(void) pmem2_source_delete(&src);

err_close:
	(void) close(fd);

err_config_delete:
	(void) pmem2_config_delete(&cfg);

err_free:
#ifdef USE_LIBPMEM
	if (is_pmem) {
		pmem_unmap(mr_ptr, mr_size);
		mr_ptr = NULL;
	}
#endif

	if (mr_ptr != NULL)
		free(mr_ptr);

	return ret;
}
