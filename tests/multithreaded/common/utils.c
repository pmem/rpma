// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2022, Intel Corporation */

/*
 * utils.c -- helper functions for multithreaded tests
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <librpma.h>

#include "mtt.h"
#include "utils.h"

#define MAX_CONN_RETRY 10

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

void
get_private_data(void *prestate, struct mtt_result *tr)
{
	struct prestate *pr = (struct prestate *)prestate;
	struct ibv_context *ibv_ctx;
	struct rpma_conn_req *req = NULL;
	enum rpma_conn_event conn_event = RPMA_CONN_UNDEFINED;

	int ret;

	if ((ret = rpma_utils_get_ibv_context(pr->addr,
			RPMA_UTIL_IBV_CONTEXT_REMOTE, &ibv_ctx))) {
		MTT_RPMA_ERR(tr, "rpma_utils_get_ibv_context", ret);
		return;
	}

	if ((ret = rpma_peer_new(ibv_ctx, &pr->peer))) {
		MTT_RPMA_ERR(tr, "rpma_peer_new", ret);
		return;
	}

	MTT_PORT_INIT;
	MTT_PORT_SET(pr->port, 0);

	int retry = 0;
	do {
		/* create a connection request */
		ret = rpma_conn_req_new(pr->peer, pr->addr, MTT_PORT_STR,
					NULL, &req);
		if (ret) {
			MTT_RPMA_ERR(tr, "rpma_conn_req_new", ret);
			goto err_peer_delete;
		}

		/*
		 * Connect the connection request and obtain
		 * the connection object.
		 */
		ret = rpma_conn_req_connect(&req, NULL, &pr->conn);
		if (ret) {
			(void) rpma_conn_req_delete(&req);
			MTT_RPMA_ERR(tr, "rpma_conn_req_connect", ret);
			goto err_peer_delete;
		}

		/* wait for the connection to establish */
		ret = rpma_conn_next_event(pr->conn, &conn_event);
		if (ret) {
			MTT_RPMA_ERR(tr, "rpma_conn_next_event", ret);
			goto err_conn_delete;
		}

		if (conn_event == RPMA_CONN_ESTABLISHED)
			break;

		retry++;

		if (conn_event != RPMA_CONN_REJECTED ||
		    retry == MAX_CONN_RETRY) {
			MTT_ERR_MSG(tr,
				"rpma_conn_next_event returned an unexpected event",
				-1);
			goto err_conn_delete;
		}

		/* received the RPMA_CONN_REJECTED event, retrying ... */
		(void) rpma_conn_disconnect(pr->conn);
		(void) rpma_conn_delete(&pr->conn);

	} while (retry < MAX_CONN_RETRY);

	/* get the connection private data */
	ret = rpma_conn_get_private_data(pr->conn, &pr->pdata_exp);
	if (ret) {
		MTT_RPMA_ERR(tr, "rpma_conn_get_private_data", ret);
		goto err_conn_delete;
	} else if (pr->pdata_exp.ptr == NULL) {
		MTT_ERR_MSG(tr,
			"The server has not provided the connection's private data",
			-1);
		goto err_conn_delete;
	}

	return;

err_conn_delete:
	(void) rpma_conn_delete(&pr->conn);

err_peer_delete:
	(void) rpma_peer_delete(&pr->peer);
}
