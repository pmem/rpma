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

#ifdef USE_LIBPMEM
#include <libpmem.h>
#define USAGE_STR "usage: %s <server_address> <port> [<pmem-path>]\n"PMEM_USAGE
#define LOG_HDR_SIGNATURE "LOG"
#else
#define USAGE_STR "usage: %s <server_address> <port>\n"
#endif /* USE_LIBPMEM */

#define LOG_SIGNATURE_SIZE 8
#define LOG_DATA_SIZE 1024

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
	void *mr_ptr = NULL;
	size_t mr_size = 0;
	struct rpma_mr_local *mr = NULL;

	struct log *log;

	int is_pmem = 0;

#ifdef USE_LIBPMEM
	char *pmem_path = NULL;
	if (argc >= 4) {
		pmem_path = argv[3];

		/* map the file */
		mr_ptr = pmem_map_file(pmem_path, 0 /* len */, 0 /* flags */,
				0 /* mode */, &mr_size, &is_pmem);
		if (mr_ptr == NULL) {
			(void) fprintf(stderr, "pmem_map_file() for %s "
					"failed\n", pmem_path);
			return -1;
		}

		/* pmem is expected */
		if (!is_pmem) {
			(void) fprintf(stderr, "%s is not an actual PMEM\n",
				pmem_path);
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
		if (mr_size < LOG_SIGNATURE_SIZE) {
			(void) fprintf(stderr, "%s too small (%zu < %u)\n",
					pmem_path, mr_size, LOG_SIGNATURE_SIZE);
			(void) pmem_unmap(mr_ptr, mr_size);
			return -1;
		}

		/*
		 * All of the space under the offset is intended for
		 * the string contents. Space is assumed to be at least 1 KiB.
		 */
		if (mr_size - LOG_SIGNATURE_SIZE < KILOBYTE) {
			fprintf(stderr, "%s too small (%zu < %u)\n",
					pmem_path, mr_size,
					KILOBYTE + LOG_SIGNATURE_SIZE);
			(void) pmem_unmap(mr_ptr, mr_size);
			return -1;
		}

		log = mr_ptr;

		/*
		 * If the signature is not in place the persistent content has
		 * to be initialized and persisted.
		 */
		if (strncmp(mr_ptr, LOG_HDR_SIGNATURE, LOG_SIGNATURE_SIZE)) {
			/* initialize used value and persist it */
			log->used = offsetof(struct log, data);
			pmem_persist(&log->used, sizeof(uint64_t));
			/* write the signature to mark the content as valid */
			strncpy(mr_ptr, LOG_HDR_SIGNATURE, LOG_SIGNATURE_SIZE);
			pmem_persist(mr_ptr, LOG_SIGNATURE_SIZE);
		}
	}
#endif /* USE_LIBPMEM */

	/* if no pmem support or it is not provided */
	if (mr_ptr == NULL) {
		(void) fprintf(stderr, NO_PMEM_MSG);
		log = malloc_aligned(sizeof(struct log));
		if (log == NULL)
			return -1;

		log->used = offsetof(struct log, data);

		mr_ptr = (void *)log;
		mr_size = sizeof(struct log);
	}

	/* RPMA resources */
	struct rpma_peer *peer = NULL;
	struct rpma_ep *ep = NULL;
	struct rpma_conn *conn = NULL;

	/*
	 * lookup an ibv_context via the address and create a new peer using it
	 */
	if ((ret = server_peer_via_address(addr, &peer)))
		goto err_free;

	/* start a listening endpoint at addr:port */
	if ((ret = rpma_ep_listen(peer, addr, port, &ep)))
		goto err_peer_delete;

	/* register the memory */
	if ((ret = rpma_mr_reg(peer, mr_ptr, mr_size,
			RPMA_MR_USAGE_WRITE_DST | RPMA_MR_USAGE_READ_SRC |
			(is_pmem ? RPMA_MR_USAGE_FLUSH_TYPE_PERSISTENT :
				RPMA_MR_USAGE_FLUSH_TYPE_VISIBILITY),
			&mr)))
		goto err_ep_shutdown;

#ifdef USE_LIBPMEM
	/* rpma_mr_advise() should be called only in case of FsDAX */
	if (is_pmem && strstr(pmem_path, "/dev/dax") == NULL) {
		ret = rpma_mr_advise(mr, 0, mr_size,
			IBV_ADVISE_MR_ADVICE_PREFETCH_WRITE,
			IBV_ADVISE_MR_FLAG_FLUSH);
		if (ret)
			goto err_mr_dereg;
	}
#endif /* USE_LIBPMEM */

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
	 * Wait for an incoming connection request, accept it and wait for its
	 * establishment.
	 */
	struct rpma_conn_private_data pdata;
	pdata.ptr = &data;
	pdata.len = sizeof(struct common_data);
	if ((ret = server_accept_connection(ep, NULL, &pdata, &conn)))
		goto err_mr_dereg;

	/*
	 * Wait for RPMA_CONN_CLOSED, disconnect and delete the connection
	 * structure.
	 */
	if ((ret = common_wait_for_conn_close_and_disconnect(&conn)))
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
#ifdef USE_LIBPMEM
	if (is_pmem) {
		pmem_unmap(mr_ptr, mr_size);
		mr_ptr = NULL;
	}
#endif /* USE_LIBPMEM */

	if (!is_pmem)
		free(log);

	return ret ? -2 : 0;
}
