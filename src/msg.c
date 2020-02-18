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
 * msg.c -- entry points for librpma MSG
 */

#include <errno.h>

#include "alloc.h"
#include "connection.h"
#include "memory.h"
#include "rpma_utils.h"
#include "util.h"

static int
msg_queue_init(struct rpma_connection *conn, size_t queue_length, int access,
	       struct rpma_memory_local **buff)
{
	size_t buff_size = conn->zone->msg_size * queue_length;
	buff_size = ALIGN_UP(buff_size, Pagesize);

	void *ptr;
	errno = posix_memalign((void **)&ptr, Pagesize, buff_size);
	if (errno)
		return RPMA_E_ERRNO;

	int ret = rpma_memory_local_new_internal(conn->zone, ptr, buff_size,
						 access, buff);
	if (ret)
		goto err_mem_local_new;

	return 0;

err_mem_local_new:
	Free(ptr);
	return ret;
}

/* XXX make it common with raw_buffer_init/_fini ? */
static int
msg_queue_fini(struct rpma_connection *conn, struct rpma_memory_local **buff)
{
	void *ptr;
	int ret = rpma_memory_local_get_ptr(*buff, &ptr);
	if (ret)
		return ret;

	ret = rpma_memory_local_delete(buff);
	if (ret)
		return ret;

	Free(ptr);

	return 0;
}

static void
msg_init(struct ibv_send_wr *send, struct ibv_recv_wr *recv,
	 struct ibv_sge *sge, struct rpma_memory_local *buff, size_t length)
{
	ASSERT(length < UINT32_MAX);
	ASSERTne(sge, NULL);

	if (send) {
		ASSERTeq(recv, NULL);

		memset(send, 0, sizeof(*send));
		send->wr_id = 0;
		send->next = NULL;
		send->sg_list = sge;
		send->num_sge = 1;
		send->opcode = IBV_WR_SEND;
		send->send_flags = IBV_SEND_SIGNALED;
	} else {
		ASSERTne(recv, NULL);

		memset(recv, 0, sizeof(*recv));
		recv->next = NULL;
		recv->sg_list = sge;
		recv->num_sge = 1;
		recv->wr_id = 0;
	}

	/* sge->addr has to be provided just before ibv_post_send */
	sge->length = (uint32_t)length;
	sge->lkey = buff->mr->lkey;
}

int
rpma_connection_msg_init(struct rpma_connection *conn)
{
	struct rpma_msg *send = &conn->send;
	struct rpma_msg *recv = &conn->recv;

	int ret;

	conn->send_buff_id = 0;

	int msg_access = IBV_ACCESS_LOCAL_WRITE; /* XXX ? */
	ret = msg_queue_init(conn, conn->zone->send_queue_length, msg_access,
			     &send->buff);
	if (ret)
		return ret;

	ret = msg_queue_init(conn, conn->zone->recv_queue_length, msg_access,
			     &recv->buff);
	if (ret)
		goto err_recv_queue_init;

	/* initialize msgs */
	msg_init(&send->send, NULL, &send->sge, send->buff,
		 conn->zone->msg_size);
	msg_init(NULL, &recv->recv, &recv->sge, recv->buff,
		 conn->zone->msg_size);

	return 0;

err_recv_queue_init:
	(void)msg_queue_fini(conn, &conn->send.buff);
	return ret;
}

int
rpma_connection_msg_fini(struct rpma_connection *conn)
{
	int ret;
	ret = msg_queue_fini(conn, &conn->recv.buff);
	if (ret)
		return ret;

	ret = msg_queue_fini(conn, &conn->send.buff);
	if (ret)
		return ret;

	return 0;
}

int
rpma_msg_get_ptr(struct rpma_connection *conn, void **ptr)
{
	void *buff;
	int ret = rpma_memory_local_get_ptr(conn->send.buff, &buff);
	if (ret)
		return ret;

	uint64_t buff_id = conn->send_buff_id;
	conn->send_buff_id = (buff_id + 1) % conn->zone->send_queue_length;

	buff = (void *)((uintptr_t)buff + buff_id * conn->zone->msg_size);

	memset(buff, 0, conn->zone->msg_size);

	*ptr = buff;

	return 0;
}

int
rpma_connection_send(struct rpma_connection *conn, void *ptr)
{
	struct ibv_send_wr *bad_wr;
	uint64_t addr = (uint64_t)ptr;

	struct rpma_msg *msg = &conn->send;
	msg->send.wr_id = addr;
	msg->sge.addr = addr;

	int ret = ibv_post_send(conn->id->qp, &msg->send, &bad_wr);
	if (ret) {
		ERR_STR(ret, "ibv_post_send");
		return ret;
	}

	ret = rpma_connection_cq_wait(conn, IBV_WC_SEND, addr);
	if (ret)
		return ret;

	return 0;
}

int
rpma_connection_recv_post(struct rpma_connection *conn, void *ptr)
{
	struct ibv_recv_wr *bad_wr;
	uint64_t addr = (uint64_t)ptr;

	struct rpma_msg *msg = &conn->recv;
	msg->recv.wr_id = addr;
	msg->sge.addr = addr;

	int ret = ibv_post_recv(conn->id->qp, &msg->recv, &bad_wr);
	if (ret)
		return -ret; /* XXX macro? */

	return 0;
}
