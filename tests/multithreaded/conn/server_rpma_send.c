// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020-2021, Intel Corporation */

/*
 * server_rpma_send.c -- implementation of rpma_send-specific functions for common server
 *                        of multi-connection MT tests
 */

#include "server_rpma_common.h"

/*
 * client_handle_completion -- callback on completion is ready
 */
void
client_handle_completion(struct custom_event *ce)
{
	struct client_res *clnt = (struct client_res *)ce->arg;
	const struct server_res *svr = clnt->svr;

	/* wait for the completion to be ready */
	int ret = rpma_cq_wait(clnt->cq);
	if (ret) {
		/* no completion is ready - continue */
		if (ret == RPMA_E_NO_COMPLETION)
			return;

		/* another error occurred - disconnect */
		(void) rpma_conn_disconnect(clnt->conn);
		return;
	}

	/* get next completion */
	struct ibv_wc wc;
	ret = rpma_cq_get_wc(clnt->cq, 1, &wc, NULL);
	if (ret) {
		/* no completion is ready - continue */
		if (ret == RPMA_E_NO_COMPLETION)
			return;

		/* another error occurred - disconnect */
		(void) rpma_conn_disconnect(clnt->conn);
		return;
	}

	/* validate received completion */
	if (wc.status != IBV_WC_SUCCESS) {
		SERVER_ERR_MSG("rpma_recv() failed");
		SERVER_ERR_MSG(ibv_wc_status_str(wc.status));
		(void) rpma_conn_disconnect(clnt->conn);
		return;
	}

	if (wc.opcode != IBV_WC_RECV) {
		SERVER_ERR_MSG("received unexpected wc.opcode value");
		(void) rpma_conn_disconnect(clnt->conn);
		return;
	}

	/* post next RDMA recv operation */
	ret = rpma_recv(clnt->conn, svr->mr_local_ptr, 0, svr->mr_local_size, NULL);
	if (ret)
		SERVER_RPMA_ERR("rpma_recv() failed", ret);
}

/*
 * client_handle_is_ready -- callback on connection is established
 */
void
client_handle_is_ready(struct client_res *clnt)
{
	const struct server_res *svr = clnt->svr;

	/* post an RDMA recv operation */
	int ret = rpma_recv(clnt->conn, svr->mr_local_ptr, 0, svr->mr_local_size, NULL);
	if (ret)
		SERVER_RPMA_ERR("rpma_recv() failed", ret);
}
