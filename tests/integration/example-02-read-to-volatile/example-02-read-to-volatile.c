/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2020, Intel Corporation
 */

/*
 * example-02-read-to-volatile.c -- 'read to volatile' integration tests
 */

#include <rdma/rdma_cma.h>
#include <stdlib.h>
#include <stdio.h>

#include "cmocka_headers.h"
#include "librpma.h"
#include "mocks.h"

#define HELLO_STR "Hello client!"
#define MOCK_READ_ADDR (&msg)
#define MOCK_READ_LEN (strlen(msg) + 1)

static const char msg[] = HELLO_STR;

struct private_data_buffer {
	uint64_t raddr;
	uint64_t size;
	uint32_t rkey;
	enum rpma_mr_plt plt;
};

/* global mocks */
static struct rdma_cm_id Cm_id;		/* mock CM ID */
static struct ibv_context Ibv_context;	/* mock IBV context */
static struct ibv_qp Ibv_qp;		/* mock IBV QP */

/* arguments for mock of posix_memalign() */
static struct posix_memalign_args Allocated;

int client_main(int argc, char *argv[]);
int server_main(int argc, char *argv[]);

/* tests */

/*
 * test_client_success -- 'read to volatile' integration test, client
 */
void
test_client_success(void **unused)
{
	/* configure mocks for rpma_utils_get_ibv_context */
	struct rdma_addrinfo res1 = {0};
	will_return(rdma_getaddrinfo, MOCK_OK);
	will_return(rdma_getaddrinfo, &res1);
	will_return_always(__wrap__test_malloc, MOCK_OK);

	will_return(__wrap_posix_memalign, &Allocated);

	will_return(rdma_create_id, &Cm_id);

	struct ibv_post_send_mock_args args_post_send;
	args_post_send.qp = &Ibv_qp;
	args_post_send.opcode = IBV_WR_RDMA_READ;
	args_post_send.send_flags = IBV_SEND_SIGNALED;
	args_post_send.wr_id = 0; /* op_context */
	args_post_send.src_addr = (uint64_t)MOCK_READ_ADDR;
	args_post_send.pdst_addr = (uint64_t *)&Allocated.ptr;
	args_post_send.dst_len = MOCK_READ_LEN;
	args_post_send.ret = MOCK_OK;
	will_return(ibv_post_send_mock, &args_post_send);

	expect_value(rdma_resolve_addr, id, &Cm_id);
	expect_value(rdma_resolve_addr, src_addr, NULL);
	expect_value(rdma_resolve_addr, dst_addr, NULL);
	expect_value(rdma_resolve_addr, timeout_ms, MOCK_TIMEOUT);
	will_return(rdma_resolve_addr, MOCK_VERBS);
	will_return(rdma_resolve_addr, MOCK_OK);

	expect_value(rdma_destroy_id, id, &Cm_id);
	will_return(rdma_destroy_id, MOCK_OK);

	expect_value(rdma_freeaddrinfo, res, &res1);

	/* configure mocks for rpma_peer_new */
	expect_value(ibv_alloc_pd, ibv_ctx, MOCK_VERBS);
	will_return(ibv_alloc_pd, MOCK_IBV_PD);

	/* configure mocks for rpma_conn_req_new() */
	struct rdma_addrinfo res2 = {0};
	will_return(rdma_getaddrinfo, MOCK_OK);
	will_return(rdma_getaddrinfo, &res2);
	will_return(rdma_getaddrinfo, MOCK_SRC_ADDR);
	will_return(rdma_getaddrinfo, MOCK_DST_ADDR);

	will_return(rdma_create_id, &Cm_id);

	expect_value(rdma_resolve_addr, id, &Cm_id);
	expect_value(rdma_resolve_addr, src_addr, MOCK_SRC_ADDR);
	expect_value(rdma_resolve_addr, dst_addr, MOCK_DST_ADDR);
	expect_value(rdma_resolve_addr, timeout_ms, MOCK_TIMEOUT);
	will_return(rdma_resolve_addr, MOCK_VERBS);
	will_return(rdma_resolve_addr, MOCK_OK);

	expect_value(rdma_resolve_route, id, &Cm_id);
	will_return(rdma_resolve_route, MOCK_OK);

	Ibv_context.ops.post_send = ibv_post_send_mock;
	Ibv_context.ops.poll_cq = ibv_poll_cq_mock;
	Ibv_cq.context = &Ibv_context;
	Ibv_qp.context = &Ibv_context;

	will_return(ibv_create_cq, &Ibv_cq);

	expect_value(rdma_create_qp, id, &Cm_id);
	expect_value(rdma_create_qp, pd, MOCK_IBV_PD);
	will_return(rdma_create_qp, MOCK_OK);
	will_return(rdma_create_qp, &Ibv_qp);

	/* configure mocks for ibv_reg_mr() */
	struct ibv_reg_mr_args args_reg_mr;
	args_reg_mr.pd = MOCK_IBV_PD;
	args_reg_mr.paddr = &Allocated.ptr;
	args_reg_mr.length = MOCK_SIZE;
	args_reg_mr.access = IBV_ACCESS_LOCAL_WRITE;
	args_reg_mr.mr = MOCK_MR;
	will_return(ibv_reg_mr, &args_reg_mr);

	expect_value(rdma_freeaddrinfo, res, &res2);

	/* configure mocks for rpma_conn_req_connect() */
	will_return(rdma_create_event_channel, MOCK_EVCH);

	expect_value(rdma_migrate_id, id, &Cm_id);
	expect_value(rdma_migrate_id, channel, MOCK_EVCH);
	will_return(rdma_migrate_id, MOCK_OK);

	expect_value(rdma_connect, id, &Cm_id);
	will_return(rdma_connect, MOCK_OK);

	/* configure mocks for rpma_conn_next_event() */
	struct private_data_buffer buff;
	buff.raddr = (uintptr_t)MOCK_READ_ADDR;
	buff.size = MOCK_READ_LEN;
	buff.rkey = MOCK_RKEY;
	buff.plt = RPMA_MR_PLT_VOLATILE;

	struct rdma_cm_event f_event = {0};
	f_event.event = RDMA_CM_EVENT_ESTABLISHED;
	f_event.param.conn.private_data = &buff;
	f_event.param.conn.private_data_len = sizeof(buff);

	expect_value(rdma_get_cm_event, channel, MOCK_EVCH);
	will_return(rdma_get_cm_event, &f_event);

	expect_value(rdma_ack_cm_event, event, &f_event);
	will_return(rdma_ack_cm_event, MOCK_OK);

	struct ibv_wc wc = {0};
	wc.opcode = IBV_WC_RDMA_READ;
	expect_value(ibv_poll_cq_mock, cq, &Ibv_cq);
	will_return(ibv_poll_cq_mock, 1);
	will_return(ibv_poll_cq_mock, &wc);

	/* configure mocks for rpma_conn_next_event() */
	struct rdma_cm_event s_event = {0};
	s_event.event = RDMA_CM_EVENT_DISCONNECTED;
	expect_value(rdma_get_cm_event, channel, MOCK_EVCH);
	will_return(rdma_get_cm_event, &s_event);

	expect_value(rdma_ack_cm_event, event, &s_event);
	will_return(rdma_ack_cm_event, MOCK_OK);

	/* configure mocks for rpma_conn_disconnect() */
	expect_value(rdma_disconnect, id, &Cm_id);
	will_return(rdma_disconnect, MOCK_OK);

	/* configure mocks for rpma_conn_delete() */
	expect_value(rdma_destroy_qp, id, &Cm_id);

	expect_value(ibv_destroy_cq, cq, &Ibv_cq);
	will_return(ibv_destroy_cq, MOCK_OK);

	expect_value(rdma_destroy_id, id, &Cm_id);
	will_return(rdma_destroy_id, MOCK_OK);

	will_return(ibv_dereg_mr, MOCK_OK);

	expect_value(rdma_destroy_event_channel, channel, MOCK_EVCH);

	/* configure mocks for rpma_peer_delete() */
	expect_value(ibv_dealloc_pd, pd, MOCK_IBV_PD);
	will_return(ibv_dealloc_pd, MOCK_OK);

	/* run test */
	char *argv[] = {"client", MOCK_ADDR, MOCK_SERVICE};
	int ret = client_main(3, argv);

	assert_int_equal(ret, MOCK_OK);
}

/*
 * test_server_success -- 'read to volatile' integration test, server
 */
void
test_server_success(void **unused)
{
	/* configure mocks for rpma_utils_get_ibv_context */
	struct rdma_addrinfo res1 = {0};
	will_return(rdma_getaddrinfo, MOCK_OK);
	will_return(rdma_getaddrinfo, &res1);
	will_return_always(__wrap__test_malloc, MOCK_OK);

	will_return(__wrap_posix_memalign, &Allocated);

	struct rdma_cm_id Cm_id = {0};
	will_return(rdma_create_id, &Cm_id);

	expect_value(rdma_bind_addr, id, &Cm_id);
	expect_value(rdma_bind_addr, addr, NULL);
	will_return(rdma_bind_addr, MOCK_VERBS);
	will_return(rdma_bind_addr, MOCK_OK);

	expect_value(rdma_destroy_id, id, &Cm_id);
	will_return(rdma_destroy_id, MOCK_OK);

	expect_value(rdma_freeaddrinfo, res, &res1);

	/* configure mocks for rpma_peer_new */
	expect_value(ibv_alloc_pd, ibv_ctx, MOCK_VERBS);
	will_return(ibv_alloc_pd, MOCK_IBV_PD);

	/* configure mocks for rpma_ep_listen() */
	will_return(rdma_create_event_channel, MOCK_EVCH);

	will_return(rdma_create_id, &Cm_id);

	struct rdma_addrinfo res2 = {0};
	will_return(rdma_getaddrinfo, MOCK_OK);
	will_return(rdma_getaddrinfo, &res2);
	will_return(rdma_getaddrinfo, MOCK_SRC_ADDR);
	will_return(rdma_getaddrinfo, MOCK_DST_ADDR);

	expect_value(rdma_bind_addr, id, &Cm_id);
	expect_value(rdma_bind_addr, addr, MOCK_SRC_ADDR);
	will_return(rdma_bind_addr, MOCK_VERBS);
	will_return(rdma_bind_addr, MOCK_OK);

	expect_value(rdma_listen, id, &Cm_id);
	will_return(rdma_listen, MOCK_OK);

	expect_value(rdma_freeaddrinfo, res, &res2);


	/* configure mocks for ibv_reg_mr() */
	struct ibv_reg_mr_args args_reg_mr;
	args_reg_mr.pd = MOCK_IBV_PD;
	args_reg_mr.paddr = &Allocated.ptr;
	args_reg_mr.length = MOCK_READ_LEN;
	args_reg_mr.access = IBV_ACCESS_REMOTE_READ;
	args_reg_mr.mr = MOCK_MR;
	will_return(ibv_reg_mr, &args_reg_mr);

	/* configure mocks for rpma_ep_next_conn_req() */
	struct rdma_cm_event f_event = {0};
	const char *msg = "Hello client!";
	f_event.param.conn.private_data = NULL;
	f_event.param.conn.private_data_len = MOCK_READ_LEN * sizeof(char);
	f_event.event = RDMA_CM_EVENT_CONNECT_REQUEST;
	f_event.id = &Cm_id;
	expect_value(rdma_get_cm_event, channel, MOCK_EVCH);
	will_return(rdma_get_cm_event, &f_event);

	will_return(ibv_create_cq, &Ibv_cq);

	expect_value(rdma_create_qp, id, &Cm_id);
	expect_value(rdma_create_qp, pd, MOCK_IBV_PD);
	will_return(rdma_create_qp, MOCK_OK);
	will_return(rdma_create_qp, &Ibv_qp);

	/* configure mocks for rpma_conn_req_connect() */
	expect_value(rdma_accept, id, &Cm_id);
	will_return(rdma_accept, MOCK_OK);

	expect_value(rdma_ack_cm_event, event, &f_event);
	will_return(rdma_ack_cm_event, MOCK_OK);

	will_return(rdma_create_event_channel, MOCK_EVCH);

	expect_value(rdma_migrate_id, id, &Cm_id);
	expect_value(rdma_migrate_id, channel, MOCK_EVCH);
	will_return(rdma_migrate_id, MOCK_OK);

	/* configure mocks for rpma_conn_next_event() */
	struct rdma_cm_event s_event = {0};
	const char *s_msg = "Hello server!";
	s_event.event = RDMA_CM_EVENT_ESTABLISHED;
	s_event.param.conn.private_data = (void *)s_msg;
	s_event.param.conn.private_data_len =
		(strlen(s_msg) + 1) * sizeof(char);

	expect_value(rdma_get_cm_event, channel, MOCK_EVCH);
	will_return(rdma_get_cm_event, &s_event);

	expect_value(rdma_ack_cm_event, event, &s_event);
	will_return(rdma_ack_cm_event, MOCK_OK);

	/* configure mocks for rpma_conn_disconnect() */
	expect_value(rdma_disconnect, id, &Cm_id);
	will_return(rdma_disconnect, MOCK_OK);

	/* configure mocks for rpma_conn_next_event() */
	struct rdma_cm_event t_event = {0};
	t_event.event = RDMA_CM_EVENT_DISCONNECTED;
	expect_value(rdma_get_cm_event, channel, MOCK_EVCH);
	will_return(rdma_get_cm_event, &t_event);

	expect_value(rdma_ack_cm_event, event, &t_event);
	will_return(rdma_ack_cm_event, MOCK_OK);

	/* configure mocks for rpma_conn_delete() */
	expect_value(rdma_destroy_qp, id, &Cm_id);

	expect_value(ibv_destroy_cq, cq, &Ibv_cq);
	will_return(ibv_destroy_cq, MOCK_OK);

	expect_value(rdma_destroy_id, id, &Cm_id);
	will_return(rdma_destroy_id, MOCK_OK);

	will_return(ibv_dereg_mr, MOCK_OK);

	expect_value(rdma_destroy_event_channel, channel, MOCK_EVCH);

	/* configure mocks for rpma_ep_shutdown() */
	expect_value(rdma_destroy_id, id, &Cm_id);
	will_return(rdma_destroy_id, MOCK_OK);

	expect_value(rdma_destroy_event_channel, channel, MOCK_EVCH);

	/* configure mocks for rpma_peer_delete() */
	expect_value(ibv_dealloc_pd, pd, MOCK_IBV_PD);
	will_return(ibv_dealloc_pd, MOCK_OK);

	/* run test */
	char *argv[] = {"server", MOCK_ADDR, MOCK_SERVICE};
	int ret = server_main(3, argv);

	assert_int_equal(ret, MOCK_OK);
}

int
main(int argc, char *argv[])
{
	const struct CMUnitTest tests[] = {
		cmocka_unit_test(test_client_success),
		cmocka_unit_test(test_server_success),
	};

	return cmocka_run_group_tests(tests, NULL, NULL);
}
