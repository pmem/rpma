// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2022, Fujitsu */
/* Copyright 2022, Intel Corporation */

/*
 * server.c -- a server of the messages-ping-pong-with-srq example
 *
 * Please see README.md for a detailed description of this example.
 */

#include <inttypes.h>
#include <librpma.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/epoll.h>

#include "common-conn.h"
#include "common-messages-ping-pong.h"
#include "common-epoll.h"

#define USAGE_STR	"usage: %s <server_address> <port> [<timeout>]\n"
#define CLIENT_MAX	10
#define TIMEOUT_5S	5000

struct connect_context {
	struct rpma_conn *conn;
	uint32_t qp_num;
};

static struct connect_context conn_ctxs[CLIENT_MAX];
static int num_clients;

static int
find_qp(uint32_t qp_num)
{
	int i;

	if (num_clients == 1)
		return 0;

	for (i = 0; i < num_clients; ++i)
		if (conn_ctxs[i].qp_num == qp_num)
			return i;

	(void) fprintf(stderr, "Unable to find the connection [qp#%d]\n", qp_num);
	return -1;
}

static int
add_fd_to_epoll(int epoll_fd, int ep_fd)
{
	struct epoll_event event;
	event.events = EPOLLIN;

	/* set O_NONBLOCK flag for the provided fd */
	if (fd_set_nonblock(ep_fd))
		return -1;

	if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, ep_fd, &event)) {
		perror("epoll_ctl(EPOLL_CTL_ADD)");
		return -1;
	}

	return 0;
}

static int
handle_incoming_connections(struct rpma_ep *ep, struct rpma_conn_cfg *cfg,
	struct rpma_mr_local *recv_mr, uint64_t *recv)
{
	size_t i, offset;
	struct connect_context *conn_ctx = NULL;
	struct rpma_srq *srq = NULL;
	int ret;

	/* find the first free slot */
	for (i = 0; i < CLIENT_MAX; ++i) {
		conn_ctx = &conn_ctxs[i];
		if (conn_ctx->conn != NULL)
			continue;

		break;
	}

	/* conn_ctx == NULL means that no free slot is found when i reaches CLIENT_MAX */
	if (conn_ctx == NULL) {
		(void) fprintf(stderr, "No free slot for a new connection request.\n");
		return -1;
	}

	offset = i;

	/* get the shared RQ object from the connection configuration */
	ret = rpma_conn_cfg_get_srq(cfg, &srq);
	if (ret)
		return ret;

	/*
	 * Put an initial receive for a connection to be prepared for
	 * the first message of the client's ping-pong.
	 */
	ret = rpma_srq_recv(srq, recv_mr, offset * MSG_SIZE, MSG_SIZE, recv + offset);
	if (ret)
		return ret;

	/*
	 * Wait for an incoming connection request,
	 * accept it and wait for its establishment.
	 */
	ret = server_accept_connection(ep, cfg, NULL, &conn_ctx->conn);
	if (ret)
		return ret;

	/* get the qp_num of each connection */
	ret = rpma_conn_get_qp_num(conn_ctx->conn, &conn_ctx->qp_num);
	if (ret)
		return ret;

	return 0;
}

int
main(int argc, char *argv[])
{
	/* validate parameters */
	if (argc < 3) {
		fprintf(stderr, USAGE_STR, argv[0]);
		exit(-1);
	}

	/* configure logging thresholds to see more details */
	rpma_log_set_threshold(RPMA_LOG_THRESHOLD, RPMA_LOG_LEVEL_INFO);
	rpma_log_set_threshold(RPMA_LOG_THRESHOLD_AUX, RPMA_LOG_LEVEL_INFO);

	/* read common parameters */
	char *addr = argv[1];
	char *port = argv[2];
	int ret;

	int timeout = argv[3] ? atoi(argv[3]) * 1000 : TIMEOUT_5S;
	if (timeout <= 0) {
		(void) fprintf(stderr,
				"<timeout> should be a positive number of seconds (%s given)\n",
				argv[3]);
		return -1;
	}

	/* prepare memory */
	struct rpma_mr_local *recv_mr, *send_mr;
	size_t size = CLIENT_MAX * MSG_SIZE;
	uint64_t *recv = malloc_aligned(size);
	if (recv == NULL)
		return -1;

	uint64_t *send = malloc_aligned(size);
	if (send == NULL) {
		ret = -1;
		goto err_free_recv;
	}

	/* RPMA resources */
	struct rpma_peer *peer = NULL;
	struct rpma_srq *srq = NULL;
	struct rpma_conn_cfg *cfg = NULL;
	struct rpma_ep *ep = NULL;
	struct rpma_cq *rcq = NULL;
	int i, offset, index, num_got, total_cnt = 0;
	int ep_fd, epoll_fd;
	struct epoll_event event;
	uint64_t *recv_ptr = NULL;
	struct ibv_wc wc[CLIENT_MAX];

	/*
	 * lookup an ibv_context via the address and create a new peer using it
	 */
	ret = server_peer_via_address(addr, &peer);
	if (ret)
		goto err_free_send;

	/* create a shared RQ object */
	ret = rpma_srq_new(peer, NULL, &srq);
	if (ret)
		goto err_peer_delete;

	/* create a new connection configuration */
	ret = rpma_conn_cfg_new(&cfg);
	if (ret)
		goto err_srq_delete;

	/* set the shared RQ object for the connection configuration */
	ret = rpma_conn_cfg_set_srq(cfg, srq);
	if (ret)
		goto err_conn_cfg_delete;

	/* start a listening endpoint at addr:port */
	ret = rpma_ep_listen(peer, addr, port, &ep);
	if (ret)
		goto err_conn_cfg_delete;

	/* register the memory */
	ret = rpma_mr_reg(peer, recv, size, RPMA_MR_USAGE_RECV, &recv_mr);
	if (ret)
		goto err_ep_shutdown;

	ret = rpma_mr_reg(peer, send, size, RPMA_MR_USAGE_SEND, &send_mr);
	if (ret)
		goto err_recv_mr_dereg;

	/* create an epoll file descriptor */
	epoll_fd = epoll_create1(EPOLL_CLOEXEC);
	if (epoll_fd == -1) {
		perror("epoll_create1(EPOLL_CLOEXEC)");
		ret = -1;
		goto err_send_mr_dereg;
	}

	/* get the endpoint's event file descriptor and add it to epoll */
	ret = rpma_ep_get_fd(ep, &ep_fd);
	if (ret)
		goto err_close_epoll_fd;

	ret = add_fd_to_epoll(epoll_fd, ep_fd);
	if (ret)
		goto err_close_epoll_fd;

	/* process epoll's events */
	while ((ret = epoll_wait(epoll_fd, &event, 1, timeout)) == 1) {
		ret = handle_incoming_connections(ep, cfg, recv_mr, recv);
		if (ret)
			goto err_conn_disconnect;

		num_clients++;
		(void) printf("The client #%i has been connected.\n", num_clients);
	}

	(void) printf("Server ended listening.\n");

	/* get the receive CQ of rpma_srq object */
	ret = rpma_srq_get_rcq(srq, &rcq);
	if (ret)
		goto err_conn_disconnect;

	while (!ret && total_cnt < num_clients) {
		/* wait for the completion to be ready */
		ret = rpma_cq_wait(rcq);
		if (ret)
			break;

		ret = rpma_cq_get_wc(rcq, num_clients, wc, &num_got);
		if (ret) {
			/* lack of completion is not an error */
			if (ret == RPMA_E_NO_COMPLETION) {
				ret = 0;
				continue;
			}

			break;
		}

		/* validate received completions */
		for (i = 0; i < num_got; ++i) {
			if (wc[i].status != IBV_WC_SUCCESS) {
				(void) fprintf(stderr, "rpma_srq_recv() failed: %s\n",
						ibv_wc_status_str(wc[i].status));
				ret = -1;
				break;
			}

			if (wc[i].opcode != IBV_WC_RECV) {
				(void) fprintf(stderr,
						"received unexpected wc.opcode value (%d != %d)\n",
						wc[i].opcode, IBV_WC_RECV);
				ret = -1;
				break;
			}

			index = find_qp(wc[i].qp_num);
			if (index == -1) {
				ret = -1;
				break;
			}

			recv_ptr = (uint64_t *)wc[i].wr_id;
			offset = recv_ptr - recv;

			if (*recv_ptr == I_M_DONE) {
				total_cnt++;
				continue;
			}

			(void) printf("[server] value received: %" PRIu64
					" from the connection [qp#%d]\n",
					*recv_ptr, conn_ctxs[index].qp_num);

			/* prepare a receive for the client's request */
			ret = rpma_srq_recv(srq, recv_mr, (size_t)offset * MSG_SIZE, MSG_SIZE,
					recv_ptr);
			if (ret)
				break;

			*(send + offset) = *recv_ptr + 1;

			(void) printf("[server] value sent: %" PRIu64
					" from the connection [qp#%d]\n",
					*(send + offset), conn_ctxs[index].qp_num);

			/* send a message to the client */
			ret = rpma_send(conn_ctxs[index].conn, send_mr, (size_t)offset * MSG_SIZE,
					MSG_SIZE, RPMA_F_COMPLETION_ON_ERROR, NULL);
			if (ret)
				break;
		}
	}

	if (total_cnt != num_clients) {
		(void) fprintf(stderr, "Unable to receive data from all connections\n");
		ret = -1;
	}

err_conn_disconnect:
	/* disconnect all remaining client's */
	for (i = 0; i < CLIENT_MAX; ++i) {
		if (conn_ctxs[i].conn == NULL)
			continue;

		(void) rpma_conn_disconnect(conn_ctxs[i].conn);
		(void) rpma_conn_delete(&conn_ctxs[i].conn);
	}

	ret |= epoll_ctl(epoll_fd, EPOLL_CTL_DEL, ep_fd, NULL);

err_close_epoll_fd:
	ret |= close(epoll_fd);
	epoll_fd = -1;

err_send_mr_dereg:
	ret |= rpma_mr_dereg(&send_mr);

err_recv_mr_dereg:
	ret |= rpma_mr_dereg(&recv_mr);

err_ep_shutdown:
	ret |= rpma_ep_shutdown(&ep);

err_conn_cfg_delete:
	ret |= rpma_conn_cfg_delete(&cfg);

err_srq_delete:
	ret |= rpma_srq_delete(&srq);

err_peer_delete:
	/* delete the peer object */
	ret |= rpma_peer_delete(&peer);

err_free_send:
	free(send);

err_free_recv:
	free(recv);

	return ret ? -1 : 0;
}
