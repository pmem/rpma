/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2020, Intel Corporation
 */

/*
 * read-server.c -- a server of the read example
 */

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <librpma.h>

#define HELLO_STR "Hello client!"

static void
print_error(const char *fname, const int ret)
{
	int result = 0;

	if (ret == RPMA_E_PROVIDER) {
		int errnum = rpma_err_get_provider_error();
		const char *errstr = strerror(errnum);
		result = fprintf(stderr, "%s failed: %s (%s)\n", fname,
				rpma_err_2str(ret), errstr);
	} else {
		result = fprintf(stderr, "%s failed: %s\n", fname,
				rpma_err_2str(ret));
	}

	if (result < 0)
		exit(-1);
}

static void *
malloc_aligned(size_t size)
{
	long pagesize = sysconf(_SC_PAGESIZE);
	if (pagesize < 0) {
		perror("sysconf");
		exit(-1);
	}

	/* allocate a page size aligned local memory pool */
	void *mem;
	int ret = posix_memalign(&mem, (size_t)pagesize, size);
	if (ret) {
		fprintf(stderr, "posix_memalign: %s\n", strerror(ret));
		exit(-1);
	}

	return mem;
}

int
main(int argc, char *argv[])
{
	if (argc < 3) {
		fprintf(stderr, "usage: %s <addr> <service>\n", argv[0]);
		abort();
	}

	/* parameters */
	char *addr = argv[1];
	char *service = argv[2];

	/* resources */
	struct ibv_context *dev = NULL;
	struct rpma_peer *peer = NULL;
	void *mr_ptr = NULL;
	size_t mr_size = 0;
	struct rpma_mr_local *mr = NULL;
	struct rpma_ep *ep = NULL;
	struct rpma_conn_req *req = NULL;
	struct rpma_conn *conn = NULL;
	enum rpma_conn_event conn_event = RPMA_CONN_UNDEFINED;
	int ret = 0;

	/* obtain an IBV context for a local IP address */
	ret = rpma_utils_get_ibv_context(addr, RPMA_UTIL_IBV_CONTEXT_LOCAL,
			&dev);
	if (ret) {
		print_error("rpma_utils_get_ibv_context", ret);
		return -1;
	}

	/* create a new peer object */
	ret = rpma_peer_new(dev, &peer);
	if (ret) {
		print_error("rpma_peer_new", ret);
		return -1;
	}

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
	ret = rpma_mr_serialize(mr, pdata.ptr);
	if (ret) {
		print_error("rpma_mr_serialize", ret);
		goto err_mr_dereg;
	}

	/* create a new endpoint object */
	ret = rpma_ep_listen(peer, addr, service, &ep);
	if (ret) {
		print_error("rpma_ep_listen", ret);
		goto err_mr_dereg;
	}
	fprintf(stdout, "Waiting for incoming connections...\n");

	/* receive an incoming connection request */
	ret = rpma_ep_next_conn_req(ep, &req);
	if (ret) {
		print_error("rpma_ep_next_conn_req", ret);
		goto err_ep_shutdown;
	}

	/*
	 * connect / accept the connection request and obtain the connection
	 * object
	 */
	ret = rpma_conn_req_connect(&req, &pdata, &conn);
	if (ret) {
		print_error("rpma_conn_req_connect", ret);
		goto err_req_delete;
	}

	/* wait for the connection to being establish */
	ret = rpma_conn_next_event(conn, &conn_event);
	if (ret) {
		print_error("rpma_conn_next_event", ret);
		goto err_conn_delete;
	} else if (conn_event != RPMA_CONN_ESTABLISHED) {
		fprintf(stderr, "rpma_conn_next_event returned an unexptected "
				"event: %s\n",
				rpma_utils_conn_event_2str(conn_event));
		goto err_conn_delete;
	} else {
		fprintf(stderr, "rpma_conn_next_event returned an event: %s\n",
				rpma_utils_conn_event_2str(conn_event));
	}

	/*
	 * between the connection being established and the connection is closed
	 * the client will perform the RDMA read
	 */

	/* wait for the connection to being closed */
	ret = rpma_conn_next_event(conn, &conn_event);
	if (ret) {
		print_error("rpma_conn_next_event", ret);
		goto err_conn_delete;
	} else if (conn_event != RPMA_CONN_CLOSED) {
		fprintf(stderr, "rpma_conn_next_event returned an unexptected "
				"event: %s\n",
				rpma_utils_conn_event_2str(conn_event));
		goto err_conn_delete;
	} else {
		fprintf(stderr, "rpma_conn_next_event returned an event: %s\n",
				rpma_utils_conn_event_2str(conn_event));
	}

	/* disconnect the connection */
	ret = rpma_conn_disconnect(conn);
	if (ret) {
		print_error("rpma_conn_disconnect", ret);
		goto err_conn_delete;
	}

	/* delete the connection object */
	ret = rpma_conn_delete(&conn);
	if (ret) {
		print_error("rpma_conn_delete", ret);
		goto err_ep_shutdown;
	}

	/* shutdown the endpoint */
	ret = rpma_ep_shutdown(&ep);
	if (ret) {
		print_error("rpma_ep_shutdown", ret);
		goto err_peer_delete;
	}

	/* deregister the memory region */
	ret = rpma_mr_dereg(&mr);
	if (ret) {
		print_error("rpma_mr_dereg", ret);
		goto err_mr_free;
	}

	/* free the memory */
	free(mr_ptr);

	/* delete the peer object */
	ret = rpma_peer_delete(&peer);
	if (ret) {
		print_error("rpma_peer_delete", ret);
		goto err_exit;
	}

	return 0;

err_conn_delete:
	(void) rpma_conn_delete(&conn);
err_req_delete:
	if (req)
		(void) rpma_conn_req_delete(&req);
err_ep_shutdown:
	(void) rpma_ep_shutdown(&ep);
err_mr_dereg:
	(void) rpma_mr_dereg(&mr);
err_mr_free:
	free(mr_ptr);
err_peer_delete:
	(void) rpma_peer_delete(&peer);

err_exit:
	return -1;
}
