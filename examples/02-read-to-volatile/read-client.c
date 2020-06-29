/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2020, Intel Corporation
 */

/*
 * read-client.c -- a client of the read example
 */

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <librpma.h>

#define KILOBYTE 1024

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
		fprintf(stderr, "usage: %s <addr> <service>\n",
			argv[0]);
		exit(-1);
	}

	/* parameters */
	char *addr = argv[1];
	char *service = argv[2];

	/* resources */
	struct ibv_context *dev = NULL;
	struct rpma_peer *peer = NULL;
	void *dst_ptr = NULL;
	struct rpma_mr_local *dst = NULL;
	struct rpma_mr_remote *src = NULL;
	size_t src_size = 0;
	struct rpma_conn_req *req = NULL;
	struct rpma_conn *conn = NULL;
	struct rpma_completion cmpl;
	enum rpma_conn_event conn_event = RPMA_CONN_UNDEFINED;
	int ret = 0;

	/* obtain an IBV context for a remote IP address */
	ret = rpma_utils_get_ibv_context(addr, RPMA_UTIL_IBV_CONTEXT_REMOTE,
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
	dst_ptr = malloc_aligned(KILOBYTE);
	if (dst_ptr == NULL) {
		ret = -1;
		goto err_peer_delete;
	}

	/* register the memory */
	ret = rpma_mr_reg(peer, dst_ptr, KILOBYTE, RPMA_MR_USAGE_READ_DST,
			RPMA_MR_PLT_VOLATILE, &dst);
	if (ret) {
		print_error("rpma_mr_reg", ret);
		goto err_mr_free;
	}

	/* create a connection request */
	ret = rpma_conn_req_new(peer, addr, service, &req);
	if (ret) {
		print_error("rpma_conn_req_new", ret);
		goto err_mr_dereg;
	}

	/* connect the connection request and obtain the connection object */
	ret = rpma_conn_req_connect(&req, NULL, &conn);
	if (ret) {
		print_error("rpma_conn_req_connect", ret);
		goto err_req_delete;
	}

	/* wait for the connection to establish */
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

	/* receive a memory info from the server */
	struct rpma_conn_private_data pdata;
	ret = rpma_conn_get_private_data(conn, &pdata);
	if (ret) {
		print_error("rpma_conn_next_event", ret);
		goto err_conn_disconnect;
	} else if (pdata.ptr == NULL) {
		print_error("No data received.", ret);
		goto err_conn_disconnect;
	}

	/* deserialize the remote memory registration */
	ret = rpma_mr_deserialize(pdata.ptr, rpma_mr_serialize_get_size(),
			&src);
	if (ret) {
		print_error("rpma_mr_deserialize", ret);
		goto err_conn_disconnect;
	}

	/* read the remote memory region size */
	ret = rpma_mr_remote_size(src, &src_size);
	if (ret) {
		print_error("rpma_mr_remote_size", ret);
		goto err_mr_remote_delete;
	}

	/* post an RDMA read operation */
	ret = rpma_read(conn, dst, 0, src, 0, src_size, RPMA_OP_FLAG_COMPLETION,
			NULL);

	/* wait for a completion of the RDMA read */
	ret = rpma_conn_next_completion(conn, &cmpl);
	if (ret) {
		print_error("rpma_conn_next_completion", ret);
		goto err_mr_remote_delete;
	} else if (cmpl.op == RPMA_OP_READ) {
		fprintf(stderr, "rpma_conn_next_completion returned a "
				"completion of an unexpected operation: %d\n",
				cmpl.op);
		goto err_mr_remote_delete;
	} else if (cmpl.op_status != IBV_WC_SUCCESS) {
		fprintf(stderr, "rpma_conn_next_completion returned an "
				"unexpected operation status: %d\n",
				cmpl.op_status);
		goto err_mr_remote_delete;
	}

	fprintf(stdout, "Read a message: %s\n", (char *)dst_ptr);

	/* delete the remote memory region object */
	ret = rpma_mr_remote_delete(&src);
	if (ret) {
		print_error("rpma_mr_remote_delete", ret);
		goto err_conn_disconnect;
	}

	/* disconnect the connection */
	ret = rpma_conn_disconnect(conn);
	if (ret) {
		print_error("rpma_conn_disconnect", ret);
		goto err_conn_delete;
	}

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

	/* delete the connection object */
	ret = rpma_conn_delete(&conn);
	if (ret) {
		print_error("rpma_conn_delete", ret);
		goto err_mr_dereg;
	}

	/* deregister the memory region */
	ret = rpma_mr_dereg(&dst);
	if (ret) {
		print_error("rpma_mr_dereg", ret);
		goto err_mr_free;
	}

	/* free the memory */
	free(dst_ptr);

	/* delete the peer object */
	ret = rpma_peer_delete(&peer);
	if (ret) {
		print_error("rpma_peer_delete", ret);
		goto err_exit;
	}

	return 0;

err_mr_remote_delete:
	(void) rpma_mr_remote_delete(&src);
err_conn_disconnect:
	(void) rpma_conn_disconnect(conn);
err_conn_delete:
	(void) rpma_conn_delete(&conn);
err_req_delete:
	if (req)
		(void) rpma_conn_req_delete(&req);
err_mr_dereg:
	(void) rpma_mr_dereg(&dst);
err_mr_free:
	free(dst_ptr);
err_peer_delete:
	(void) rpma_peer_delete(&peer);

err_exit:
	return -1;
}
