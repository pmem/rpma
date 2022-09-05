// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2022, Intel Corporation */

/*
 * rpma_mr_remote_from_descriptor.c -- rpma_mr_remote_from_descriptor multithreaded test
 */

#include <stdlib.h>
#include <librpma.h>

#include "mtt.h"
#include "mtt_connect.h"

/* the client's part */

struct prestate {
	char *addr;
	unsigned port;
	struct rpma_peer *peer;
	struct rpma_conn *conn;
	struct rpma_mr_remote *mr_ptr;
	size_t mr_size;

	/* the expected value of the private data */
	struct rpma_conn_private_data pdata;
};

static void
prestate_init(void *prestate, struct mtt_result *tr)
{
	struct prestate *pr = (struct prestate *)prestate;

	int ret;

	if (mtt_client_peer_new(tr, pr->addr, &pr->peer))
		return;

	if (mtt_client_connect(tr, pr->addr, pr->port, pr->peer, &pr->conn, &pr->pdata))
		goto err_peer_delete;

	/*
	 * Create a remote memory registration structure from the received
	 * descriptor.
	 */
	struct common_data *dst_data = pr->pdata.ptr;

	ret = rpma_mr_remote_from_descriptor(&dst_data->descriptors[0], dst_data->mr_desc_size,
			&pr->mr_ptr);
	if (ret) {
		MTT_RPMA_ERR(tr, "rpma_mr_remote_from_descriptor", ret);
		goto err_conn_disconnect;
	};

	ret = rpma_mr_remote_get_size(pr->mr_ptr, &pr->mr_size);
	if (ret) {
		MTT_RPMA_ERR(tr, "rpma_mr_remote_get_size", ret);
		goto err_mr_remote_delete;
	};

	return;

err_mr_remote_delete:
	/* delete the remote memory region's structure */
	(void) rpma_mr_remote_delete(&pr->mr_ptr);

err_conn_disconnect:
	mtt_client_err_disconnect(&pr->conn);

err_peer_delete:
	mtt_client_peer_delete(tr, &pr->peer);
}

/*
 * thread -- get and verify the private data
 */
static void
thread(unsigned id, void *prestate, void *state, struct mtt_result *result)
{
	struct prestate *pr = (struct prestate *)prestate;
	struct rpma_mr_remote *mr_ptr;
	struct common_data *dst_data = pr->pdata.ptr;

	int ret = rpma_mr_remote_from_descriptor(&dst_data->descriptors[0], dst_data->mr_desc_size,
				&mr_ptr);
	if (ret) {
		MTT_RPMA_ERR(result, "rpma_mr_remote_from_descriptor", ret);
		return;
	} else if (mr_ptr == NULL) {
		MTT_ERR_MSG(result, "Getting mr_remote from descriptor failed", -1);
		return;
	}

	if (memcmp(mr_ptr, pr->mr_ptr, pr->mr_size) != 0)
		MTT_ERR_MSG(result, "Wrong content of the mr_remote", -1);

	ret = rpma_mr_remote_delete(&mr_ptr);
	if (ret)
		MTT_RPMA_ERR(result, "rpma_mr_remote_delete", ret);
}

/*
 * prestate_fini -- deregister and free the memory region, disconnect and delete the peer object
 */
static void
prestate_fini(void *prestate, struct mtt_result *tr)
{
	struct prestate *pr = (struct prestate *)prestate;
	enum rpma_conn_event conn_event = RPMA_CONN_UNDEFINED;
	int ret;

	ret = rpma_conn_disconnect(pr->conn);
	if (ret) {
		MTT_RPMA_ERR(tr, "rpma_conn_disconnect", ret);
	} else {
		/* wait for the connection to be closed */
		ret = rpma_conn_next_event(pr->conn, &conn_event);
		if (ret)
			MTT_RPMA_ERR(tr, "rpma_conn_next_event", ret);
		else if (conn_event != RPMA_CONN_CLOSED)
			MTT_ERR_MSG(tr, "rpma_conn_next_event returned an unexpected event", -1);
	}

	ret = rpma_mr_remote_delete(&pr->mr_ptr);
	if (ret)
		MTT_RPMA_ERR(tr, "rpma_mr_remote_delete", ret);

	ret = rpma_conn_delete(&pr->conn);
	if (ret)
		MTT_RPMA_ERR(tr, "rpma_conn_delete", ret);

	ret = rpma_peer_delete(&pr->peer);
	if (ret)
		MTT_RPMA_ERR(tr, "rpma_peer_delete", ret);
}

/* the server's part */

struct server_prestate {
	char *addr;
	unsigned port;

	/* the expected value of the private data */
	struct rpma_conn_private_data pdata;
};

/*
 * server_main -- the main function of the server
 */
int
server_main(char *addr, unsigned port);

/*
 * server_func -- the server function of this test
 */
int
server_func(void *prestate)
{
	struct server_prestate *pst = (struct server_prestate *)prestate;
	return server_main(pst->addr, pst->port);
}

int
main(int argc, char *argv[])
{
	struct mtt_args args = {0};

	if (mtt_parse_args(argc, argv, &args))
		return -1;

	struct prestate client_prestate = {args.addr, args.port};
	struct server_prestate server_prestate = {args.addr, args.port};

	struct mtt_test test = {
			&client_prestate,
			prestate_init,
			NULL,
			NULL,
			thread,
			NULL,
			NULL,
			prestate_fini,
			server_func,
			&server_prestate
	};

	return mtt_run(&test, args.threads_num);
}
