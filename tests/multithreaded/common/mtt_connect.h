/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright 2022, Intel Corporation */

/*
 * mtt_connect.h -- header for common connection code of multithreaded tests
 */

#ifndef MTT_CONNECT_H
#define MTT_CONNECT_H

#define DESCRIPTORS_MAX_SIZE 24

int mtt_server_listen(char *addr, unsigned port, struct rpma_peer **peer_ptr,
		struct rpma_ep **ep_ptr);

int mtt_server_accept_connection(struct rpma_ep *ep,
		struct rpma_conn_private_data *pdata,
		struct rpma_conn **conn_ptr);

void mtt_server_wait_for_conn_close_and_disconnect(struct rpma_conn **conn_ptr);

void mtt_server_shutdown(struct rpma_peer **peer_ptr, struct rpma_ep **ep_ptr);

int mtt_client_connect(struct mtt_result *tr, char *addr, unsigned port,
		struct rpma_peer **peer_ptr, struct rpma_conn **conn_ptr,
		struct rpma_conn_private_data *pdata);

void mtt_client_err_disconnect(struct rpma_conn **conn_ptr,
		struct rpma_peer **peer_ptr);

void mtt_client_disconnect(struct mtt_result *tr, struct rpma_conn **conn_ptr,
		struct rpma_peer **peer_ptr);

#endif /* MTT_CONNECT_H */
