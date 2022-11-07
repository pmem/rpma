// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020-2022, Intel Corporation */

/*
 * server.c -- a server of the atomic-write example
 *
 * Please see README.md for a detailed description of this example.
 */

#include <inttypes.h>
#include <librpma.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "common-conn.h"
#include "common-pmem_map_file.h"

#ifdef USE_PMEM
#define USAGE_STR "usage: %s <server_address> <port> [<pmem-path>]\n"PMEM_USAGE
#else
#define USAGE_STR "usage: %s <server_address> <port>\n"
#endif /* USE_PMEM */

#ifdef USE_PMEM
#define LOG_HDR_SIGNATURE "LOG"
#endif /* USE_PMEM */

#define LOG_SIGNATURE_SIZE 8
#define LOG_DATA_SIZE 1024
#define LOG_SIZE (sizeof(struct log))

/* defined log structure */
struct log {
	char signature[LOG_SIGNATURE_SIZE];
	/* last written data (aligned to RPMA_ATOMIC_WRITE_ALIGNMENT) */
	uint64_t used;
	char data[LOG_DATA_SIZE];
};

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
	struct rpma_mr_local *mr = NULL;

	struct log *log = NULL;

#ifdef USE_PMEM
	char *pmem_path = NULL;
	if (argc >= 4) {
		pmem_path = argv[3];

		ret = common_pmem_map_file(pmem_path, LOG_SIZE, &mem);
		if (ret)
			goto err_free;

		log = (struct log *)mem.mr_ptr;

		/*
		 * If the signature is not in place the persistent content has to be initialized
		 * and persisted.
		 */
		if (strncmp(mem.mr_ptr, LOG_HDR_SIGNATURE, LOG_SIGNATURE_SIZE)) {
			/* initialize used value and persist it */
			log->used = offsetof(struct log, data);
			mem.persist(&log->used, sizeof(uint64_t));
			/* write the signature to mark the content as valid */
			strncpy(mem.mr_ptr, LOG_HDR_SIGNATURE, LOG_SIGNATURE_SIZE);
			mem.persist(mem.mr_ptr, LOG_SIGNATURE_SIZE);
		}
	}
#endif /* USE_PMEM */

	/* if no pmem support or it is not provided */
	if (mem.mr_ptr == NULL) {
		(void) fprintf(stderr, NO_PMEM_MSG);
		mem.mr_ptr = malloc_aligned(LOG_SIZE);
		if (mem.mr_ptr == NULL)
			return -1;

		mem.mr_size = LOG_SIZE;

		log = (struct log *)mem.mr_ptr;
		log->used = offsetof(struct log, data);
	}

	/* RPMA resources */
	struct rpma_peer *peer = NULL;
	struct rpma_ep *ep = NULL;
	struct rpma_conn *conn = NULL;

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
	ret = rpma_mr_reg(peer, mem.mr_ptr, mem.mr_size,
			RPMA_MR_USAGE_WRITE_DST | RPMA_MR_USAGE_READ_SRC |
			(mem.is_pmem ? RPMA_MR_USAGE_FLUSH_TYPE_PERSISTENT :
				RPMA_MR_USAGE_FLUSH_TYPE_VISIBILITY),
			&mr);
	if (ret)
		goto err_ep_shutdown;

#if defined USE_PMEM && defined IBV_ADVISE_MR_FLAGS_SUPPORTED
	/* rpma_mr_advise() should be called only in case of FsDAX */
	if (mem.is_pmem && strstr(pmem_path, "/dev/dax") == NULL) {
		ret = rpma_mr_advise(mr, 0, mem.mr_size, IBV_ADVISE_MR_ADVICE_PREFETCH_WRITE,
			IBV_ADVISE_MR_FLAG_FLUSH);
		if (ret)
			goto err_mr_dereg;
	}
#endif /* USE_PMEM */

	/* get size of the memory region's descriptor */
	size_t mr_desc_size;
	ret = rpma_mr_get_descriptor_size(mr, &mr_desc_size);
	if (ret)
		goto err_mr_dereg;

	/* calculate data for the client write */
	struct common_data data = {0};
	data.data_offset = offsetof(struct log, used);
	data.mr_desc_size = mr_desc_size;

	/* get the memory region's descriptor */
	ret = rpma_mr_get_descriptor(mr, &data.descriptors[0]);
	if (ret)
		goto err_mr_dereg;

	/*
	 * Wait for an incoming connection request, accept it and wait for its establishment.
	 */
	struct rpma_conn_private_data pdata;
	pdata.ptr = &data;
	pdata.len = sizeof(struct common_data);
	ret = server_accept_connection(ep, NULL, &pdata, &conn);
	if (ret)
		goto err_mr_dereg;

	/*
	 * Wait for RPMA_CONN_CLOSED, disconnect and delete the connection structure.
	 */
	ret = common_wait_for_conn_close_and_disconnect(&conn);
	if (ret)
		goto err_mr_dereg;

	/* print the saved data */
	char *ptr = log->data;
	char *log_data_tail = (char *)log + log->used;
	while (ptr < log_data_tail) {
		if (*ptr)
			putc(*ptr, stdout);
		else
			putc('\n', stdout);
		ptr++;
	}

err_mr_dereg:
	/* deregister the memory region */
	(void) rpma_mr_dereg(&mr);

err_ep_shutdown:
	/* shutdown the endpoint */
	(void) rpma_ep_shutdown(&ep);

err_peer_delete:
	/* delete the peer object */
	(void) rpma_peer_delete(&peer);

err_free:
#ifdef USE_PMEM
	if (mem.is_pmem) {
		common_pmem_unmap_file(&mem);
	} else
#endif /* USE_PMEM */
	if (mem.mr_ptr != NULL) {
		free(mem.mr_ptr);
	}

	return ret ? -2 : 0;
}
