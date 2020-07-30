// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020, Intel Corporation */

/*
 * server.c -- a server of the multiple-connections example
 *
 * Please see README.md for a detailed description of this example.
 */

#include <inttypes.h>
#include <librpma.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/epoll.h>
#include <unistd.h>

#define USAGE_STR "usage: %s <server_address> <service>\n"

#include "common.h"
#include "common-epoll.h"

#define CLIENT_MAX 10

struct client_res {
	/* RPMA resources */
	struct rpma_conn *conn;

	/* resources - memory regions */
	size_t offset;

	/* events */
	struct custom_event ev_conn_event;
	struct custom_event ev_conn_cmpl;

	/* XXX */
	struct server_res *svr;
	int client_id;
};

struct server_res {
	/* RPMA resources */
	struct rpma_ep *ep;

	/* resources - memory region */
	void *dst_ptr;
	struct rpma_mr_local *dst_mr;

	/* epoll and event */
	int epoll;
	struct custom_event ev_incoming;

	/* client's resources */
	struct client_res clients[CLIENT_MAX];
	int client_num;
};

/*
 * conn_cmpl -- callback on completion is ready
 *
 * The only exptected completion in this example is a success of
 * the RPMA_OP_READ after which the read client's name is printed to the output.
 * No matter what the disconnection process is initialized.
 */
void
conn_cmpl(struct custom_event *ce)
{
	struct client_res *clnt = (struct client_res *)ce->arg;
	const struct server_res *svr = clnt->svr;

	/* prepare detected completions for processing */
	int ret = rpma_conn_prepare_completions(clnt->conn);
	if (ret) {
		/* no completion is ready - continue */
		if (ret == RPMA_E_NO_COMPLETION)
			return;

		/* another error occured - disconnect */
		print_error_ex("rpma_conn_prepare_completions", ret);
		(void) rpma_conn_disconnect(clnt->conn);
		return;
	}

	/* get next completion */
	struct rpma_completion cmpl;
	ret = rpma_conn_next_completion(clnt->conn, &cmpl);
	if (ret) {
		/* no completion is ready - continue */
		if (ret == RPMA_E_NO_COMPLETION)
			return;

		/* another error occured - disconnect */
		print_error_ex("rpma_conn_next_completion", ret);
		(void) rpma_conn_disconnect(clnt->conn);
		return;
	}

	/* validate received completion */
	if (cmpl.op != RPMA_OP_READ || cmpl.op_status != IBV_WC_SUCCESS) {
		(void) fprintf(stderr,
				"[%d] received completion is not as expected (%d != %d || %d != %d)\n",
				clnt->client_id,
				cmpl.op, RPMA_OP_READ,
				cmpl.op_status, IBV_WC_SUCCESS);

		(void) rpma_conn_disconnect(clnt->conn);
		return;
	}

	/* print received name of the client */
	char *name = (char *)svr->dst_ptr + clnt->offset;
	printf("- %s\n", name);

	/* initialize disconnection process */
	ret = rpma_conn_disconnect(clnt->conn);
	if (ret)
		print_error_ex("rpma_conn_disconnect", ret);
}

/*
 * conn_event_established -- callback on connection is established
 *
 * The client should send via the private data the remote memory regions
 * descriptor which allows reading client's name.
 * If any of the required steps fail the client will be disconnected.
 */
void
conn_event_established(struct client_res *clnt)
{
	const struct server_res *svr = clnt->svr;

	/* get connection's private data */
	struct rpma_conn_private_data pdata;
	rpma_mr_descriptor *desc;
	rpma_conn_get_private_data(clnt->conn, &pdata);
	if (pdata.len < sizeof(rpma_mr_descriptor)) {
		(void) fprintf(stderr,
				"[%d] received connection's private data is too small (%d < %zu)\n",
				clnt->client_id,
				pdata.len, sizeof(rpma_mr_descriptor));
		(void) rpma_conn_disconnect(clnt->conn);
		return;
	}
	desc = (rpma_mr_descriptor *)pdata.ptr;

	/* prepare a remote memory region */
	struct rpma_mr_remote *src_mr;
	int ret = rpma_mr_remote_from_descriptor(desc, &src_mr);
	if (ret) {
		print_error_ex("rpma_mr_remote_from_descriptor", ret);
		(void) rpma_conn_disconnect(clnt->conn);
		return;
	}

	/* read client's name from the remote memory region */
	ret = rpma_read(clnt->conn, svr->dst_mr, clnt->offset,
			src_mr, 0, DWORD, RPMA_F_COMPLETION_ALWAYS, NULL);
	if (ret) {
		print_error_ex("rpma_read", ret);
		(void) rpma_mr_remote_delete(&src_mr);
		(void) rpma_conn_disconnect(clnt->conn);
	}

	/* delete the remote memory region's object */
	ret = rpma_mr_remote_delete(&src_mr);
	if (ret)
		print_error_ex("rpma_read", ret);
}

/*
 * conn_event_closed -- callback on connection is closed
 *
 * Delete:
 * - all client's file descriptors from the server's epoll
 * - the connection
 */
void
conn_event_closed(struct client_res *clnt)
{
	struct server_res *svr = clnt->svr;

	(void) epoll_delete(svr->epoll, &clnt->ev_conn_cmpl);
	(void) epoll_delete(svr->epoll, &clnt->ev_conn_event);

	/* delete the connection */
	int ret = rpma_conn_delete(&clnt->conn);
	if (ret)
		print_error_ex("rpma_conn_delete", ret);

	/* decrease the number of active connections */
	--svr->client_num;
}

/*
 * conn_event -- callback on connection's next event
 */
void
conn_event(struct custom_event *ce)
{
	struct client_res *clnt = (struct client_res *)ce->arg;

	/* get next connection's event */
	enum rpma_conn_event event;
	int ret = rpma_conn_next_event(clnt->conn, &event);
	if (ret) {
		if (ret == RPMA_E_NO_NEXT)
			return;

		print_error_ex("rpma_conn_next_event", ret);
		(void) rpma_conn_disconnect(clnt->conn);
		return;
	}

	/* proceed to the callback specific to the gotten event */
	switch (event) {
	case RPMA_CONN_ESTABLISHED:
		conn_event_established(clnt);
		break;
	case RPMA_CONN_CLOSED:
	default:
		conn_event_closed(clnt);
	}
}

/*
 * ep_incoming_func -- callback on endpoint's next incoming connection
 *
 * Get the connection request. If there is not free slots reject it. Otherwise,
 * accept the incoming connection, get the event and completion file
 * descriptors, set O_NONBLOCK flag for both of them and add events to
 * the epoll.
 * If error will occur at any of the required steps the client is disconnected.
 */
void
ep_incoming_func(struct custom_event *ce)
{
	struct server_res *svr = (struct server_res *)ce->arg;

	/* receive an incoming connection request */
	struct rpma_conn_req *req = NULL;
	int ret = rpma_ep_next_conn_req(svr->ep, &req);
	if (ret) {
		if (ret == RPMA_E_NO_NEXT)
			return;

		print_error_ex("rpma_ep_next_conn_req", ret);
		return;
	}

	/* check if at least one free slot is available */
	if (svr->client_num == CLIENT_MAX) {
		rpma_conn_req_delete(&req);
		return;
	}

	/* find the first free slot */
	struct client_res *clnt = NULL;
	for (int i = 0; i < CLIENT_MAX; ++i) {
		clnt = &svr->clients[i];
		if (clnt->conn != NULL)
			continue;

		clnt->client_id = i;
		clnt->svr = svr;
		clnt->offset = (size_t)(DWORD * i);
		break;
	}

	/* accept the connection request and obtain the connection object */
	ret = rpma_conn_req_connect(&req, NULL, &clnt->conn);
	if (ret) {
		print_error_ex("rpma_conn_req_connect", ret);
		(void) rpma_conn_req_delete(&req);
		return;
	}

	/* increase the number of active connections */
	++svr->client_num;

	/* get the connection's event file descriptor */
	int fd;
	ret = rpma_conn_get_event_fd(clnt->conn, &fd);
	if (ret) {
		print_error_ex("rpma_conn_get_event_fd", ret);
		goto err_sync_disconnect;
	}

	/* set O_NONBLOCK flag for the connection's event fd */
	ret = fd_set_nonblock(fd);
	if (ret)
		goto err_sync_disconnect;

	/* prepare a custom event structure and add it to the epoll */
	custom_event_set(&clnt->ev_conn_event, fd, clnt, conn_event);
	ret = epoll_add(svr->epoll, &clnt->ev_conn_event);
	if (ret)
		goto err_sync_disconnect;

	/* get the connection's completion file descriptor */
	ret = rpma_conn_get_completion_fd(clnt->conn, &fd);
	if (ret) {
		print_error_ex("rpma_conn_get_completion_fd", ret);
		(void) rpma_conn_disconnect(clnt->conn);
		return;
	}

	/* set O_NONBLOCK flag for the connection's completion fd */
	ret = fd_set_nonblock(fd);
	if (ret) {
		(void) rpma_conn_disconnect(clnt->conn);
		return;
	}

	/* prepare a custom event structure and add it to the epoll */
	custom_event_set(&clnt->ev_conn_cmpl, fd, clnt, conn_cmpl);
	ret = epoll_add(svr->epoll, &clnt->ev_conn_cmpl);
	if (ret)
		(void) rpma_conn_disconnect(clnt->conn);

	return;

err_sync_disconnect:
	(void) common_disconnect_and_wait_for_conn_close(&clnt->conn);
	--svr->client_num;
}

int
main(int argc, char *argv[])
{
	/* validate parameters */
	if (argc < 3) {
		fprintf(stderr, USAGE_STR, argv[0]);
		exit(-1);
	}

	/* read common parameters */
	char *addr = argv[1];
	char *service = argv[2];
	int ret;

	/* RPMA resources - general */
	struct rpma_peer *peer = NULL;

	/* server resource */
	struct server_res svr = {0};

	/* scalable I/O */
	int ep_fd;
	svr.epoll = epoll_create1(EPOLL_CLOEXEC);
	if (svr.epoll == -1)
		return errno;

	/*
	 * lookup an ibv_context via the address and create a new peer using it
	 */
	ret = server_peer_via_address(addr, &peer);
	if (ret)
		goto err_close_epoll;

	/* allocate a memory */
	size_t dst_size = DWORD * CLIENT_MAX;
	svr.dst_ptr = malloc_aligned(dst_size);
	if (svr.dst_ptr == NULL) {
		ret = -1;
		goto err_peer_delete;
	}

	/* register the memory */
	ret = rpma_mr_reg(peer, svr.dst_ptr, dst_size, RPMA_MR_USAGE_READ_DST,
			RPMA_MR_PLT_VOLATILE, &svr.dst_mr);
	if (ret) {
		print_error_ex("rpma_mr_reg", ret);
		goto err_mr_free;
	}

	/* start a listening endpoint at addr:service */
	ret = server_listen(peer, addr, service, &svr.ep);
	if (ret)
		goto err_mr_dereg;

	/* get the endpoint's event file descriptor */
	ret = rpma_ep_get_fd(svr.ep, &ep_fd);
	if (ret) {
		print_error_ex("rpma_ep_get_fd", ret);
		goto err_ep_shutdown;
	}

	/* set O_NONBLOCK flag for the endpoint's event fd */
	ret = fd_set_nonblock(ep_fd);
	if (ret)
		goto err_ep_shutdown;

	/* prepare a custom event structure and add it to the epoll */
	custom_event_set(&svr.ev_incoming, ep_fd, &svr, ep_incoming_func);
	ret = epoll_add(svr.epoll, &svr.ev_incoming);
	if (ret)
		goto err_ep_shutdown;

	(void) printf("Today I have got in touch with:\n");

	/* process epoll's events */
	struct epoll_event event;
	struct custom_event *ce;
	do {
		ret = epoll_wait(svr.epoll, &event, 1 /* # of events */,
				TIMEOUT_15s);
		if (ret == -1)
			break;

		if (ret == 1) {
			ce = (struct custom_event *)event.data.ptr;
			ce->func(ce);
		} else if (ret == 0) { /* timeout */
			/* disconnect all remaining client's */
			for (int i = 0; i < CLIENT_MAX; ++i) {
				if (svr.clients[i].conn == NULL)
					continue;

				(void) rpma_conn_disconnect(
						svr.clients[i].conn);
			}

			(void) fprintf(stderr, "Server timed out.\n");
			break;
		}
	} while (1);

err_ep_shutdown:
	/* shutdown the endpoint */
	ret = rpma_ep_shutdown(&svr.ep);
	if (ret)
		print_error_ex("rpma_ep_shutdown", ret);

err_mr_dereg:
	/* deregister the memory region */
	ret = rpma_mr_dereg(&svr.dst_mr);
	if (ret)
		print_error_ex("rpma_mr_dereg", ret);

err_mr_free:
	/* free the memory */
	free(svr.dst_ptr);

err_peer_delete:
	/* delete the peer object */
	ret = rpma_peer_delete(&peer);
	if (ret)
		print_error_ex("rpma_peer_delete", ret);

err_close_epoll:
	/* close the epoll */
	if (close(svr.epoll))
		return errno;

	return ret;
}
