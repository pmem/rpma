/*
 * Copyright 2019-2020, Intel Corporation
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in
 *       the documentation and/or other materials provided with the
 *       distribution.
 *
 *     * Neither the name of the copyright holder nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * dispatcher.h -- internal definitions for librpma dispatcher
 */
#ifndef RPMA_DISPATCHER_H
#define RPMA_DISPATCHER_H

#include "os_thread.h"
#include "sys/queue.h"

struct rpma_dispatcher_conn {
	PMDK_TAILQ_ENTRY(rpma_dispatcher_conn) next;

	struct rpma_connection *conn;
};

struct rpma_dispatcher_wc_entry {
	PMDK_TAILQ_ENTRY(rpma_dispatcher_wc_entry) next;

	struct rpma_connection *conn;
	struct ibv_wc wc;
};

struct rpma_dispatcher_func_entry {
	PMDK_TAILQ_ENTRY(rpma_dispatcher_func_entry) next;

	struct rpma_connection *conn;
	rpma_queue_func func;
	void *arg;
};

struct rpma_dispatcher {
	struct rpma_zone *zone;

	PMDK_TAILQ_HEAD(head_conn, rpma_dispatcher_conn) conn_set;

	uint64_t waiting;

	PMDK_TAILQ_HEAD(head_cq, rpma_dispatcher_wc_entry) queue_wce;

	os_mutex_t queue_func_mtx;
	PMDK_TAILQ_HEAD(head_fq, rpma_dispatcher_func_entry) queue_func;
};

int rpma_dispatcher_attach_connection(struct rpma_dispatcher *disp,
				      struct rpma_connection *conn);

int rpma_dispatcher_detach_connection(struct rpma_dispatcher *disp,
				      struct rpma_connection *conn);

int rpma_dispatch_break(struct rpma_dispatcher *disp);

int rpma_dispatcher_enqueue_cq_entry(struct rpma_dispatcher *disp,
				     struct rpma_connection *conn,
				     struct ibv_wc *wc);
int rpma_dispatcher_enqueue_func(struct rpma_dispatcher *disp,
				 struct rpma_connection *conn,
				 rpma_queue_func func, void *arg);

#endif /* dispatcher.h */
