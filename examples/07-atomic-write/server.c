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

#define LOG_HDR_SIGNATURE "LOG"
#define LOG_SIGNATURE_SIZE 8
#define LOG_DATA_SIZE 102400 /* 100 KiB */

/* defined log_header structure */
struct log_header {
	char signature[LOG_SIGNATURE_SIZE];
	uint64_t used; /* offset data */
};

/* defined log structure */
struct log {
	struct log_header header;
	char data[LOG_DATA_SIZE];
};

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
	size_t used_offset = 0;
	enum rpma_mr_plt mr_plt = RPMA_MR_PLT_VOLATILE;
	struct rpma_mr_local *mr = NULL;

#ifdef USE_LIBPMEM
	/* log structure in libpmem */
#endif

	size_t log_size = (sizeof(struct log_header) +
			(sizeof(char) * LOG_DATA_SIZE));
	struct log *log = malloc_aligned(log_size);
	if (log == NULL)
		return -1;

	struct log_header *header = &log->header;
	memcpy(header->signature, LOG_HDR_SIGNATURE, LOG_SIGNATURE_SIZE);
	header->used = offsetof(struct log, data);

	used_offset = offsetof(struct log_header, used);

	/* if no pmem support or it is not provided */
	if (mr_ptr == NULL) {
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

	/* calculate data for the client write */
	struct common_data data;
	data.data_offset = used_offset;

	/* get the memory region's descriptor */
	if ((ret = rpma_mr_get_descriptor(mr, &data.desc)))
		goto err_mr_dereg;

	/*
	 * Wait for an incoming connection request, accept it and wait for its
	 * establishment.
	 */
	struct rpma_conn_private_data pdata;
	pdata.ptr = &data;
	pdata.len = sizeof(struct common_data);
	if ((ret = server_accept_connection(ep, &pdata, &conn)))
		goto err_mr_dereg;

	/*
	 * Wait for RPMA_CONN_CLOSED, disconnect and delete the connection
	 * structure.
	 */
	if ((ret = common_wait_for_conn_close_and_disconnect(&conn)))
		goto err_mr_dereg;

	/* print log structure */
	printf("header->signature: %s\nheader->used: %lu\nlog->data:\n",
			log->header.signature, log->header.used);
	char *ptr = log->data;
	do {
	ptr += printf("%s\n", ptr);
	} while (*ptr);

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
	/* unmap pmem */
#endif

	if (log != NULL)
		free(log);

	return ret ? -2 : 0;
}
