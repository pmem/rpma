// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2022, Intel Corporation */

/*
 * server_rpma_mr_remote_from_descriptor.c -- a server of the rpma_mr_remote_from_descriptor MT test
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <librpma.h>

#include "mtt.h"
#include "mtt_connect.h"

#define HELLO_STR "Hello client!"

int
server_main(char *addr, unsigned port)
{
	struct rpma_peer *peer;
	struct rpma_ep *ep;
	struct rpma_conn *conn = NULL;
	int ret;

	/* resources - memory region */
	void *mr_ptr = NULL;
	size_t mr_size = 0;
	struct rpma_mr_local *mr = NULL;

	ret = mtt_server_listen(addr, port, &peer, &ep);
	if (ret)
		return -1;

	/* allocate a memory */
	mr_size = strlen(HELLO_STR) + 1;
	mr_ptr = mtt_malloc_aligned(mr_size, NULL);
	if (mr_ptr == NULL) {
		ret = -1;
		goto err_server_shutdown;
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
	ret = mtt_server_accept_connection(ep, &pdata, &conn);
	if (ret)
		goto err_server_shutdown;

	/*
	 * Wait for RPMA_CONN_CLOSED, disconnect and delete the connection
	 * structure.
	 */
	mtt_server_wait_for_conn_close_and_disconnect(&conn);

err_mr_dereg:
	/* deregister the memory region */
	(void) rpma_mr_dereg(&mr);

err_mr_free:
	/* free the memory */
	free(mr_ptr);

err_server_shutdown:
	mtt_server_shutdown(&peer, &ep);

	return ret;
}
