// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020-2022, Intel Corporation */

/*
 * server_rpma_send.c -- implementation of rpma_send-specific functions for common server
 *                        of multi-connection MT tests
 */

#include "server_rpma_conn_common.h"

/*
 * client_completion_event_handle -- callback on completion is ready
 */
void
client_completion_event_handle(struct custom_event *ce)
{
	struct client_res *clnt = (struct client_res *)ce->arg;
	const struct server_res *svr = clnt->svr;

	/* wait for the completion to be ready */
	int ret = rpma_cq_wait(clnt->cq);
	if (ret)
		goto err_cq_wait_and_get_wc;

	/* get next completion */
	struct ibv_wc wc;
	ret = rpma_cq_get_wc(clnt->cq, 1, &wc, NULL);
	if (ret)
		goto err_cq_wait_and_get_wc;

	/* validate received completion */
	if (wc.status != IBV_WC_SUCCESS) {
		SERVER_ERR_MSG("completion status is different from IBV_WC_SUCCESS");
		SERVER_ERR_MSG(ibv_wc_status_str(wc.status));
		goto err_disconnect;
	}

	if (wc.opcode != IBV_WC_RECV) {
		SERVER_ERR_MSG("received unexpected wc.opcode value");
		goto err_disconnect;
	}

	if (wc.wr_id != WR_ID_RECV) {
		SERVER_ERR_MSG("wrong work request ID");
		goto err_disconnect;
	}

	if (memcmp(svr->local_ptr, STRING_TO_WRITE_SEND, LEN_STRING_TO_WRITE_SEND) != 0)
		SERVER_ERR_MSG("sent data mismatch");

	/* post next RDMA recv operation */
	ret = rpma_recv(clnt->conn, svr->mr_local_ptr, 0, svr->mr_local_size, (void *)WR_ID_RECV);
	if (ret)
		SERVER_RPMA_ERR("rpma_recv() failed", ret);

	return;

err_cq_wait_and_get_wc:
	/* no completion is ready - continue */
	if (ret == RPMA_E_NO_COMPLETION)
		return;

err_disconnect:
	/* another error occurred - disconnect */
	(void) rpma_conn_disconnect(clnt->conn);
}

/*
 * client_is_ready_handle -- callback on connection is established
 */
void
client_is_ready_handle(struct client_res *clnt)
{
	const struct server_res *svr = clnt->svr;

	/* post an RDMA recv operation */
	int ret = rpma_recv(clnt->conn, svr->mr_local_ptr, 0, svr->mr_local_size,
				(void *)WR_ID_RECV);
	if (ret)
		SERVER_RPMA_ERR("rpma_recv() failed", ret);
}
