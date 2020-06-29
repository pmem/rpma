// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020, Intel Corporation */

/*
 * server.c -- a server of the read example
 */

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

	int ret = server_peer_via_address(addr, &peer);
	if (ret)
		return ret;

	/* allocate a memory */
	mr_size = strlen(HELLO_STR) + 1;
	mr_ptr = malloc_aligned(mr_size);
	if (mr_ptr == NULL) {
		ret = -1;
		goto err_peer_delete;
	}

	memcpy(mr_ptr, HELLO_STR, mr_size);

	/* register the memory */
	ret = rpma_mr_reg(peer, mr_ptr, mr_size, RPMA_MR_USAGE_READ_SRC,
			RPMA_MR_PLT_VOLATILE, &mr);
	if (ret) {
		print_error("rpma_mr_reg", ret);
		goto err_mr_free;
	}

	/* serialize the memory region */
	struct rpma_conn_private_data pdata;
	pdata.len = rpma_mr_serialize_get_size();
	pdata.ptr = malloc(pdata.len);
	if (pdata.ptr == NULL) {
		ret = -1;
		goto err_mr_dereg;
	}

	ret = rpma_mr_serialize(mr, pdata.ptr);
	if (ret) {
		print_error("rpma_mr_serialize", ret);
		goto err_pdata_ptr_free;
	}

	ret = server_listen(peer, addr, service, &ep);
	if (ret)
		goto err_pdata_ptr_free;

	ret = server_connect(ep, &pdata, &conn);
	if (ret)
		goto err_ep_shutdown;

	/*
	 * between the connection being established and the connection is closed
	 * the client will perform the RDMA read. server_disconnect() will wait
	 * for the client to close the connection before disconnect by themself.
	 */

	ret = server_disconnect(&conn);

err_ep_shutdown:
	/* shutdown the endpoint */
	ret = rpma_ep_shutdown(&ep);
	if (ret) {
		print_error("rpma_ep_shutdown", ret);
		goto err_peer_delete;
	}

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

err_peer_delete:
	/* delete the peer object */
	ret = rpma_peer_delete(&peer);
	if (ret)
		print_error("rpma_peer_delete", ret);

	return ret;
}
