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
 * zone.h -- internal definitions for librpma zone
 */
#ifndef RPMA_ZONE_H
#define RPMA_ZONE_H

#include <infiniband/verbs.h>
#include <librpma.h>

struct rpma_zone {
	struct rdma_addrinfo *rai;

	struct rdma_event_channel *ec;
	int ec_epoll;

	struct ibv_context *device;
	struct ibv_pd *pd;

	struct rdma_cm_id *listen_id;
	struct rdma_cm_event *edata;

	void *uarg;
	uint64_t active_connections;
	struct ravl *connections;

	uint64_t waiting;

	rpma_on_connection_event_func on_connection_event_func;
	rpma_on_timeout_func on_timeout_func;
	int timeout;

	/* XXX should be rpma_connection specific? */
	size_t msg_size;
	uint64_t send_queue_length;
	uint64_t recv_queue_length;

	unsigned flags;
};

int rpma_zone_event_ack(struct rpma_zone *zone);
int rpma_zone_wait_connected(struct rpma_zone *zone,
			     struct rpma_connection *conn);

#endif /* zone.h */
