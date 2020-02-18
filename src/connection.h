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
 * connection.h -- internal definitions for librpma connection
 */
#ifndef RPMA_CONNECTION_H
#define RPMA_CONNECTION_H

#include <infiniband/verbs.h>
#include <rdma/rdma_cma.h>

#include <librpma.h>

struct rpma_rma {
	struct rpma_memory_local *raw_dst;
	struct rpma_memory_remote *raw_src;

	struct ibv_sge sge;
	struct ibv_send_wr wr;
};

struct rpma_msg {
	struct rpma_memory_local *buff;

	struct ibv_sge sge;
	union {
		struct ibv_recv_wr recv;
		struct ibv_send_wr send;
	};
};

struct rpma_connection {
	struct rpma_zone *zone;

	struct rdma_cm_id *id;
	struct ibv_cq *cq;
	int disconnected;

	struct rpma_dispatcher *disp;

	rpma_on_transmission_notify_func on_transmission_notify_func;
	rpma_on_connection_recv_func on_connection_recv_func;

	struct rpma_rma rma;

	struct rpma_msg send;
	struct rpma_msg recv;
	uint64_t send_buff_id;

	void *custom_data;
};

int rpma_connection_rma_init(struct rpma_connection *conn);
int rpma_connection_rma_fini(struct rpma_connection *conn);

int rpma_connection_msg_init(struct rpma_connection *conn);
int rpma_connection_msg_fini(struct rpma_connection *conn);

int rpma_connection_recv_post(struct rpma_connection *conn, void *ptr);

int rpma_connection_cq_wait(struct rpma_connection *conn,
			    enum ibv_wc_opcode opcode, uint64_t wr_id);
int rpma_connection_cq_process(struct rpma_connection *conn);

int rpma_connection_cq_entry_process(struct rpma_connection *conn,
				     struct ibv_wc *wc);

#endif /* connection.h */
