// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020-2021, Intel Corporation */

/*
 * server.c -- a server of the read-to-volatile example
 *
 *  The server in this example exposes its local memory to a client and allows
 *  him reading its contents.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <librpma.h>

#include "common-conn.h"

#define HELLO_STR "Hello client!"

#ifdef TEST_USE_CMOCKA
#include "cmocka_headers.h"
#include "cmocka_alloc.h"
#endif

#ifdef TEST_MOCK_MAIN
#define main server_main
#endif

int
main(int argc, char *argv[])
{
	if (argc < 3) {
		fprintf(stderr, "usage: %s <server_address> <port>\n",
				argv[0]);
		exit(-1);
	}

	/* configure logging thresholds to see more details */
	rpma_log_set_threshold(RPMA_LOG_THRESHOLD, RPMA_LOG_LEVEL_INFO);
	rpma_log_set_threshold(RPMA_LOG_THRESHOLD_AUX, RPMA_LOG_LEVEL_INFO);

	/* parameters */
	char *addr = argv[1];
	char *port = argv[2];

	/* resources - general */
	struct rpma_peer *peer = NULL;
	struct rpma_ep *ep = NULL;
	struct rpma_conn *conn = NULL;

	/* resources - memory region */
	void *mr_ptr = NULL;
	size_t mr_size = 0;
	struct rpma_mr_local *mr = NULL;

	/*
	 * lookup an ibv_context via the address and create a new peer using it
	 */
	int ret = server_peer_via_address(addr, &peer);
	if (ret)
		return ret;

	/* start a listening endpoint at addr:port */
	ret = rpma_ep_listen(peer, addr, port, &ep);
	if (ret)
		goto err_peer_delete;

	/* allocate a memory */
	mr_size = strlen(HELLO_STR) + 1;
	mr_ptr = malloc_aligned(mr_size);
	if (mr_ptr == NULL) {
		ret = -1;
		goto err_ep_shutdown;
	}

	/* fill the memory with a content */
	memcpy(mr_ptr, HELLO_STR, mr_size);

	/* register the memory */
	ret = rpma_mr_reg(peer, mr_ptr, mr_size, RPMA_MR_USAGE_READ_SRC, &mr);
	if (ret)
		goto err_mr_free;

	/* get size of the memory region's descriptor */
	size_t mr_desc_size;
	ret = rpma_mr_get_descriptor_size(mr, &mr_desc_size);
	if (ret)
		goto err_mr_dereg;

	struct common_data data = {0};
	data.mr_desc_size = mr_desc_size;

	/* get the memory region's descriptor */
	ret = rpma_mr_get_descriptor(mr, &data.descriptors[0]);
	if (ret)
		goto err_mr_dereg;

	struct rpma_conn_private_data pdata;
	pdata.ptr = &data;
	pdata.len = sizeof(struct common_data);

	/*
	 * Wait for an incoming connection request, accept it and wait for its
	 * establishment.
	 */
	ret = server_accept_connection(ep, NULL, &pdata, &conn);
	if (ret)
		goto err_mr_dereg;

	/*
	 * Between the connection being established and the connection being
	 * closed the client will perform the RDMA read.
	 */

	/*
	 * Wait for RPMA_CONN_CLOSED, disconnect and delete the connection
	 * structure.
	 */
	(void) common_wait_for_conn_close_and_disconnect(&conn);

err_mr_dereg:
	/* deregister the memory region */
	(void) rpma_mr_dereg(&mr);

err_mr_free:
	/* free the memory */
	free(mr_ptr);

err_ep_shutdown:
	/* shutdown the endpoint */
	(void) rpma_ep_shutdown(&ep);

err_peer_delete:
	/* delete the peer object */
	(void) rpma_peer_delete(&peer);

	return ret;
}
