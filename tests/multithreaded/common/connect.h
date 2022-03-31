/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright 2022, Intel Corporation */

/*
 * connect.h -- header for common connection code of multithreaded tests
 */

int server_listen(char *addr, unsigned port, struct rpma_peer **peer_ptr,
		struct rpma_ep **ep_ptr);

int server_accept_connection(struct rpma_ep *ep,
		struct rpma_conn_private_data *pdata,
		struct rpma_conn **conn_ptr);

void server_wait_for_conn_close_and_disconnect(struct rpma_conn **conn_ptr);

void server_shutdown(struct rpma_peer **peer_ptr, struct rpma_ep **ep_ptr);

int client_connect(struct mtt_result *tr, char *addr, unsigned port,
		struct rpma_peer **peer_ptr, struct rpma_conn **conn_ptr,
		struct rpma_conn_private_data *pdata);

void client_err_disconnect(struct rpma_conn **conn_ptr,
		struct rpma_peer **peer_ptr);

void client_disconnect(struct mtt_result *tr, struct rpma_conn **conn_ptr,
		struct rpma_peer **peer_ptr);
