// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020, Intel Corporation */

/*
 * common.c -- a common functions used by examples
 */

#include <librpma.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "common.h"

/*
 * print_error -- print RPMA error to stderr
 */
void
print_error(const char *fname, int ret)
{
	if (ret == RPMA_E_PROVIDER) {
		int errnum = rpma_err_get_provider_error();
		const char *errstr = strerror(errnum);
		(void) fprintf(stderr, "%s failed: %s (%s)\n", fname,
				rpma_err_2str(ret), errstr);
	} else {
		(void) fprintf(stderr, "%s failed: %s\n", fname,
				rpma_err_2str(ret));
	}
}

/*
 * malloc_aligned -- allocate an aligned chunk of memory
 */
void *
malloc_aligned(size_t size)
{
	long pagesize = sysconf(_SC_PAGESIZE);
	if (pagesize < 0) {
		perror("sysconf");
		return NULL;
	}

	/* allocate a page size aligned local memory pool */
	void *mem;
	int ret = posix_memalign(&mem, (size_t)pagesize, size);
	if (ret) {
		(void) fprintf(stderr, "posix_memalign: %s\n", strerror(ret));
		return NULL;
	}

	return mem;
}
/*
 * common_peer_via_address -- create a new RPMA peer based on ibv_context
 * received by the provided address
 */
int
common_peer_via_address(const char *addr, enum rpma_util_ibv_context_type type,
		struct rpma_peer **peer_ptr)
{
	struct ibv_context *dev = NULL;

	int ret = rpma_utils_get_ibv_context(addr, type, &dev);
	if (ret) {
		print_error("rpma_utils_get_ibv_context", ret);
		return -1;
	}

	/* create a new peer object */
	ret = rpma_peer_new(dev, peer_ptr);
	if (ret) {
		print_error("rpma_peer_new", ret);
		return -1;
	}

	return 0;
}

/*
 * client_connect -- establish a new connection to a server listening at
 * addr:service
 */
int
client_connect(struct rpma_peer *peer, const char *addr, const char *service,
		struct rpma_conn **conn_ptr)
{
	struct rpma_conn_req *req = NULL;
	enum rpma_conn_event conn_event = RPMA_CONN_UNDEFINED;

	/* create a connection request */
	int ret = rpma_conn_req_new(peer, addr, service, &req);
	if (ret) {
		print_error("rpma_conn_req_new", ret);
		return -1;
	}

	/* connect the connection request and obtain the connection object */
	ret = rpma_conn_req_connect(&req, NULL, conn_ptr);
	if (ret) {
		print_error("rpma_conn_req_connect", ret);
		(void) rpma_conn_req_delete(&req);
		return -1;
	}

	/* wait for the connection to establish */
	ret = rpma_conn_next_event(*conn_ptr, &conn_event);
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

	return 0;

err_conn_delete:
	(void) rpma_conn_delete(conn_ptr);

	return ret;
}

/*
 * client_disconnect -- disconnect, wait for RPMA_CONN_CLOSED and delete the
 * connection structure
 */
int
client_disconnect(struct rpma_conn **conn_ptr)
{
	enum rpma_conn_event conn_event = RPMA_CONN_UNDEFINED;

	/* disconnect the connection */
	int ret = rpma_conn_disconnect(*conn_ptr);
	if (ret) {
		print_error("rpma_conn_disconnect", ret);
		goto err_conn_delete;
	}

	/* wait for the connection to being closed */
	ret = rpma_conn_next_event(*conn_ptr, &conn_event);
	if (ret) {
		print_error("rpma_conn_next_event", ret);
	} else if (conn_event != RPMA_CONN_CLOSED) {
		fprintf(stderr, "rpma_conn_next_event returned an unexptected "
				"event: %s\n",
				rpma_utils_conn_event_2str(conn_event));
	} else {
		fprintf(stderr, "rpma_conn_next_event returned an event: %s\n",
				rpma_utils_conn_event_2str(conn_event));
	}

err_conn_delete:
	/* delete the connection object */
	ret = rpma_conn_delete(conn_ptr);
	if (ret)
		print_error("rpma_conn_delete", ret);

	return ret;
}

/*
 * server_listen -- start a listening endpoint at addr:service
 */
int
server_listen(struct rpma_peer *peer, const char *addr, const char *service,
		struct rpma_ep **ep_ptr)
{
	/* create a new endpoint object */
	int ret = rpma_ep_listen(peer, addr, service, ep_ptr);
	if (ret) {
		print_error("rpma_ep_listen", ret);
		return ret;
	}
	fprintf(stdout, "Waiting for incoming connections...\n");

	return 0;
}

/*
 * server_accept_connection -- wait for an incoming connection request,
 * accept it and wait for its establishment
 */
int
server_accept_connection(struct rpma_ep *ep,
		struct rpma_conn_private_data *pdata,
		struct rpma_conn **conn_ptr)
{
	struct rpma_conn_req *req = NULL;
	enum rpma_conn_event conn_event = RPMA_CONN_UNDEFINED;

	/* receive an incoming connection request */
	int ret = rpma_ep_next_conn_req(ep, &req);
	if (ret) {
		print_error("rpma_ep_next_conn_req", ret);
		return ret;
	}

	/*
	 * connect / accept the connection request and obtain the connection
	 * object
	 */
	ret = rpma_conn_req_connect(&req, pdata, conn_ptr);
	if (ret) {
		print_error("rpma_conn_req_connect", ret);
		(void) rpma_conn_req_delete(&req);
		return ret;
	}

	/* wait for the connection to being establish */
	ret = rpma_conn_next_event(*conn_ptr, &conn_event);
	if (ret) {
		print_error("rpma_conn_next_event", ret);
	} else if (conn_event != RPMA_CONN_ESTABLISHED) {
		fprintf(stderr, "rpma_conn_next_event returned an unexptected "
				"event: %s\n",
				rpma_utils_conn_event_2str(conn_event));
		ret = -1;
	} else {
		fprintf(stdout, "rpma_conn_next_event returned an event: %s\n",
				rpma_utils_conn_event_2str(conn_event));
	}

	if (ret)
		(void) rpma_conn_delete(conn_ptr);

	return ret;
}

/*
 * server_disconnect -- wait for RPMA_CONN_CLOSED, disconnect and delete the
 * connection structure
 */
int
server_disconnect(struct rpma_conn **conn_ptr)
{
	enum rpma_conn_event conn_event = RPMA_CONN_UNDEFINED;

	/* wait for the connection to being closed */
	int ret = rpma_conn_next_event(*conn_ptr, &conn_event);
	if (ret) {
		print_error("rpma_conn_next_event", ret);
	} else if (conn_event != RPMA_CONN_CLOSED) {
		fprintf(stderr, "rpma_conn_next_event returned an unexptected "
				"event: %s\n",
				rpma_utils_conn_event_2str(conn_event));
	} else {
		fprintf(stderr, "rpma_conn_next_event returned an event: %s\n",
				rpma_utils_conn_event_2str(conn_event));
	}

	/* disconnect the connection */
	ret = rpma_conn_disconnect(*conn_ptr);
	if (ret)
		print_error("rpma_conn_disconnect", ret);

	/* delete the connection object */
	ret = rpma_conn_delete(conn_ptr);
	if (ret)
		print_error("rpma_conn_delete", ret);

	return ret;
}
