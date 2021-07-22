// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020-2021, Intel Corporation */

/*
 * common-conn.c -- a common connection functions used by examples
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "common-conn.h"

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

	/* zero the allocated memory */
	memset(mem, 0, size);

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
	if (ret)
		return ret;

	/* create a new peer object */
	return rpma_peer_new(dev, peer_ptr);
}

/*
 * client_connect -- establish a new connection to a server listening at
 * addr:port
 */
int
client_connect(struct rpma_peer *peer, const char *addr, const char *port,
		struct rpma_conn_cfg *cfg, struct rpma_conn_private_data *pdata,
		struct rpma_conn **conn_ptr)
{
	struct rpma_conn_req *req = NULL;
	enum rpma_conn_event conn_event = RPMA_CONN_UNDEFINED;

	/* create a connection request */
	int ret = rpma_conn_req_new(peer, addr, port, cfg, &req);
	if (ret)
		return ret;

	/* connect the connection request and obtain the connection object */
	ret = rpma_conn_req_connect(&req, pdata, conn_ptr);
	if (ret) {
		(void) rpma_conn_req_delete(&req);
		return ret;
	}

	/* wait for the connection to establish */
	ret = rpma_conn_next_event(*conn_ptr, &conn_event);
	if (ret) {
		goto err_conn_delete;
	} else if (conn_event != RPMA_CONN_ESTABLISHED) {
		fprintf(stderr,
			"rpma_conn_next_event returned an unexpected event: %s\n",
			rpma_utils_conn_event_2str(conn_event));
		ret = -1;
		goto err_conn_delete;
	}

	return 0;

err_conn_delete:
	(void) rpma_conn_delete(conn_ptr);

	return ret;
}

/*
 * server_accept_connection -- wait for an incoming connection request,
 * accept it and wait for its establishment
 */
int
server_accept_connection(struct rpma_ep *ep, struct rpma_conn_cfg *cfg,
		struct rpma_conn_private_data *pdata,
		struct rpma_conn **conn_ptr)
{
	struct rpma_conn_req *req = NULL;
	enum rpma_conn_event conn_event = RPMA_CONN_UNDEFINED;

	/* receive an incoming connection request */
	int ret = rpma_ep_next_conn_req(ep, cfg, &req);
	if (ret)
		return ret;

	/*
	 * connect / accept the connection request and obtain the connection
	 * object
	 */
	ret = rpma_conn_req_connect(&req, pdata, conn_ptr);
	if (ret) {
		(void) rpma_conn_req_delete(&req);
		return ret;
	}

	/* wait for the connection to be established */
	ret = rpma_conn_next_event(*conn_ptr, &conn_event);
	if (!ret && conn_event != RPMA_CONN_ESTABLISHED) {
		fprintf(stderr,
			"rpma_conn_next_event returned an unexpected event: %s\n",
			rpma_utils_conn_event_2str(conn_event));
		ret = -1;
	}

	if (ret)
		(void) rpma_conn_delete(conn_ptr);

	return ret;
}

/*
 * common_wait_for_conn_close_verbose -- wait for RPMA_CONN_CLOSED and print
 * an error message on error
 */
static inline int
common_wait_for_conn_close_verbose(struct rpma_conn *conn)
{
	enum rpma_conn_event conn_event = RPMA_CONN_UNDEFINED;

	/* wait for the connection to be closed */
	int ret = rpma_conn_next_event(conn, &conn_event);
	if (!ret && conn_event != RPMA_CONN_CLOSED) {
		fprintf(stderr,
			"rpma_conn_next_event returned an unexpected event: %s\n",
			rpma_utils_conn_event_2str(conn_event));
	}

	return ret;
}

/*
 * common_wait_for_conn_close_and_disconnect -- wait for RPMA_CONN_CLOSED,
 * disconnect and delete the connection structure
 */
int
common_wait_for_conn_close_and_disconnect(struct rpma_conn **conn_ptr)
{
	int ret = 0;
	ret |= common_wait_for_conn_close_verbose(*conn_ptr);
	ret |= rpma_conn_disconnect(*conn_ptr);
	ret |= rpma_conn_delete(conn_ptr);

	return ret;
}

/*
 * common_disconnect_and_wait_for_conn_close -- disconnect, wait for
 * RPMA_CONN_CLOSED and delete the connection structure
 */
int
common_disconnect_and_wait_for_conn_close(struct rpma_conn **conn_ptr)
{
	int ret = 0;

	ret |= rpma_conn_disconnect(*conn_ptr);
	if (ret == 0)
		ret |= common_wait_for_conn_close_verbose(*conn_ptr);

	ret |= rpma_conn_delete(conn_ptr);

	return ret;
}
