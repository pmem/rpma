// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020, Intel Corporation */

/*
 * server.c -- a server of the atomic-write example
 *
 * Please see README.md for a detailed description of this example.
 */

#include <inttypes.h>
#include <librpma.h>
#include <librpma_log.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef USE_LIBPMEM
#include <libpmem.h>
#define USAGE_STR "usage: %s <server_address> <port> [<pmem-path>]\n"
#else
#define USAGE_STR "usage: %s <server_address> <port>\n"
#endif /* USE_LIBPMEM */

#include "common-conn.h"

#ifdef USE_LIBPMEM
#define LOG_HDR_SIGNATURE "LOG"
#endif
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
	enum rpma_mr_plt mr_plt = RPMA_MR_PLT_VOLATILE;
	struct rpma_mr_local *mr = NULL;

	struct log *log;

#ifdef USE_LIBPMEM
	if (argc == 4) {
		char *path = argv[3];
		int is_pmem;

		/* map the file */
		mr_ptr = pmem_map_file(path, 0 /* len */, 0 /* flags */,
				0 /* mode */, &mr_size, &is_pmem);
		if (mr_ptr == NULL)
			return -1;

		/* pmem is expected */
		if (!is_pmem) {
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
					path, mr_size, LOG_SIGNATURE_SIZE);
			(void) pmem_unmap(mr_ptr, mr_size);
			return -1;
		}

		/*
		 * All of the space under the offset is intended for
		 * the string contents. Space is assumed to be at least 1 KiB.
		 */
		if (mr_size - LOG_SIGNATURE_SIZE < KILOBYTE) {
			fprintf(stderr, "%s too small (%zu < %u)\n",
					path, mr_size,
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

		mr_plt = RPMA_MR_PLT_PERSISTENT;
	}
#endif

	/* if no pmem support or it is not provided */
	if (mr_ptr == NULL) {
		size_t log_size = (sizeof(struct log) +
				(sizeof(char) * LOG_DATA_SIZE));
		log = malloc_aligned(log_size);
		if (log == NULL)
			return -1;

		log->used = offsetof(struct log, data);

		mr_ptr = (void *)log;
		mr_size = log_size;
		mr_plt = RPMA_MR_PLT_VOLATILE;
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
			RPMA_MR_USAGE_WRITE_DST | RPMA_MR_USAGE_READ_SRC,
			mr_plt, &mr)))
		goto err_ep_shutdown;

	/* get size of the memory region's descriptor */
	size_t mr_desc_size;
	ret = rpma_mr_get_descriptor_size(mr, &mr_desc_size);
	if (ret)
		goto err_mr_dereg;

	/* calculate data for the client write */
	size_t data_size = sizeof(struct common_data) + mr_desc_size;
	struct common_data *data = malloc(data_size);
	if (data == NULL)
		goto err_mr_dereg;

	data->mr_desc_offset = 0;
	data->mr_desc_size = mr_desc_size;

	/* get the memory region's descriptor */
	ret = rpma_mr_get_descriptor(mr,
			&data->descriptors[data->mr_desc_offset]);
	if (ret)
		goto err_free_data;

	/*
	 * Wait for an incoming connection request, accept it and wait for its
	 * establishment.
	 */
	struct rpma_conn_private_data pdata;
	pdata.ptr = data;
	pdata.len = data_size;
	if ((ret = server_accept_connection(ep, &pdata, &conn)))
		goto err_free_data;

	/*
	 * Wait for RPMA_CONN_CLOSED, disconnect and delete the connection
	 * structure.
	 */
	if ((ret = common_wait_for_conn_close_and_disconnect(&conn)))
		goto err_free_data;

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

err_free_data:
	free(data);

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
	if (mr_plt == RPMA_MR_PLT_PERSISTENT) {
		pmem_unmap(mr_ptr, mr_size);
		mr_ptr = NULL;
	}
#endif

	if (mr_plt == RPMA_MR_PLT_VOLATILE)
		free(log);

	return ret ? -2 : 0;
}
