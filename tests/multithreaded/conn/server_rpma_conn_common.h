/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright 2020-2022, Intel Corporation */

/*
 * server_rpma_conn_common.h -- definitions of common server of multi-connection MT tests
 */

#include <inttypes.h>
#include <librpma.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/epoll.h>
#include <unistd.h>

#include "mtt.h"
#include "mtt_connect.h"
#include "mtt_common-epoll.h"
#include "rpma_conn_common.h"

#define CLIENT_MAX 32

struct client_res {
	/* RPMA resources */
	struct rpma_conn *conn;
	struct rpma_cq *cq;

	/* events */
	struct custom_event ev_conn_event;
	struct custom_event ev_conn_cmpl;

	/* parent and identifier */
	struct server_res *svr;
	int client_id;
};

struct server_res {
	/* RPMA resources */
	struct rpma_ep *ep;

	/* resources - memory region */
	void *local_ptr;
	struct rpma_mr_local *mr_local_ptr;
	size_t mr_local_size;

	struct common_data data;
	struct rpma_conn_private_data pdata;

	/* epoll and event */
	int epoll;
	struct custom_event ev_incoming;

	/* client's resources */
	struct client_res clients[CLIENT_MAX];
};

/*
 * client_completion_event_handle -- callback on completion is ready (test-specific)
 */
void client_completion_event_handle(struct custom_event *ce);

/*
 * client_connection_event_handle -- callback on connection's next event
 */
void client_connection_event_handle(struct custom_event *ce);

/*
 * client_is_ready_handle -- callback on connection is established (test-specific)
 */
void client_is_ready_handle(struct client_res *clnt);
