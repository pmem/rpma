// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020, Intel Corporation */

/*
 * server.c -- a server of the read-to-volatile example
 *
 *  The server in this example exposes its local memory to a client and allows
 *  him reading its contents.
 */

#include <inttypes.h>
#include <librpma.h>
#include <stdlib.h>
#include <stdio.h>

#include "common.h"

#define HELLO_STR "Hello client!"

int
main(int argc, char *argv[])
{
	if (argc < 3) {
		fprintf(stderr, "usage: %s <server_address> <service>\n",
				argv[0]);
		exit(-1);
	}

	/* parameters */
	char *addr = argv[1];
	char *service = argv[2];

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

	/* start a listening endpoint at addr:service */
	ret = server_listen(peer, addr, service, &ep);
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
	ret = rpma_mr_reg(peer, mr_ptr, mr_size, RPMA_MR_USAGE_READ_SRC,
			RPMA_MR_PLT_VOLATILE, &mr);
	if (ret) {
		print_error("rpma_mr_reg", ret);
		goto err_mr_free;
	}

	/* prepare a buffer for serialized memory region information */
	struct rpma_conn_private_data pdata;
	pdata.len = rpma_mr_serialize_get_size();
	pdata.ptr = malloc(pdata.len);
	if (pdata.ptr == NULL) {
		ret = -1;
		goto err_mr_dereg;
	}

	/* serialize the memory region */
	ret = rpma_mr_serialize(mr, pdata.ptr);
	if (ret) {
		print_error("rpma_mr_serialize", ret);
	} else {
		fprintf(stdout, "Serialized memory region structure size: %" PRIu8 "\n",
				pdata.len);
	}

	/*
	 * wait for an incoming connection request, accept it and wait for its
	 * establishment
	 */
	ret = server_accept_connection(ep, &pdata, &conn);
	if (ret)
		goto err_pdata_ptr_free;

	/*
	 * between the connection being established and the connection is closed
	 * the client will perform the RDMA read.
	 */

	/*
	 * wait for RPMA_CONN_CLOSED, disconnect and delete the connection
	 * structure
	 */
	ret = server_disconnect(&conn);

err_pdata_ptr_free:
	free(pdata.ptr);

err_mr_dereg:
	/* deregister the memory region */
	ret = rpma_mr_dereg(&mr);
	if (ret)
		print_error("rpma_mr_dereg", ret);

err_mr_free:
	/* free the memory */
	free(mr_ptr);

err_ep_shutdown:
	/* shutdown the endpoint */
	ret = rpma_ep_shutdown(&ep);
	if (ret) {
		print_error("rpma_ep_shutdown", ret);
		goto err_peer_delete;
	}

err_peer_delete:
	/* delete the peer object */
	ret = rpma_peer_delete(&peer);
	if (ret)
		print_error("rpma_peer_delete", ret);

	return ret;
}
