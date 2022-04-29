/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright 2020-2022, Intel Corporation */

/*
 * server_rpma_common.h -- definitions of common server of multi-connection MT tests
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
#include "rpma_rw_common.h"

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

	struct common_data data;
	struct rpma_conn_private_data pdata;

	/* epoll and event */
	int epoll;
	struct custom_event ev_incoming;

	/* client's resources */
	struct client_res clients[CLIENT_MAX];
};

/*
 * client_handle_completion -- callback on completion is ready (test-specific)
 */
void client_handle_completion(struct custom_event *ce);

/*
 * client_handle_is_ready -- callback on connection is established (test-specific)
 */
void client_handle_is_ready(struct client_res *clnt);
