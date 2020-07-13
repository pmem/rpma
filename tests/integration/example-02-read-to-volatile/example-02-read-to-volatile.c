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

int client_main(int argc, char *argv[]);
int server_main(int argc, char *argv[]);

/* tests */

/*
 * test_client -- connection integration test, client
 */
void
test_client(void **unused)
{
	/* configure mocks for rpma_utils_get_ibv_context */
	struct rdma_addrinfo res1 = {0};
	will_return(rdma_getaddrinfo, MOCK_OK);
	will_return(rdma_getaddrinfo, &res1);
	will_return_always(__wrap__test_malloc, MOCK_OK);

	Ibv_context.ops.post_send = ibv_post_send_mock;
	Ibv_qp.context = &Ibv_context;
	Cm_id.qp = &Ibv_qp;
	will_return(rdma_create_id, &Cm_id);

	struct ibv_post_send_mock_args args;
	args.qp = &Ibv_qp;
	args.opcode = IBV_WR_RDMA_READ;
	args.send_flags = IBV_SEND_SIGNALED;
	args.wr_id = 0; /* op_context */
	args.ret = MOCK_OK;
	will_return(ibv_post_send_mock, &args);

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

	Ibv_context.ops.poll_cq = ibv_poll_cq_mock;
	Ibv_cq.context = &Ibv_context;
	will_return(ibv_create_cq, &Ibv_cq);

	expect_value(rdma_create_qp, id, &Cm_id);
	expect_value(rdma_create_qp, pd, MOCK_IBV_PD);
	will_return(rdma_create_qp, MOCK_OK);

	/* configure mocks for ibv_reg_mr() */
	expect_value(ibv_reg_mr, pd, MOCK_IBV_PD);
	expect_value(ibv_reg_mr, length, MOCK_SIZE);
	expect_value(ibv_reg_mr, access, IBV_ACCESS_LOCAL_WRITE);
	will_return(ibv_reg_mr, MOCK_MR);

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
	buff.raddr = (uintptr_t)MOCK_SRC_ADDR;
	buff.size = MOCK_SIZE;
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
 * test_server -- connection integration test, server
 */
void
test_server(void **unused)
{
	/* XXX TBD WIP */
}

int
main(int argc, char *argv[])
{
	const struct CMUnitTest tests[] = {
		cmocka_unit_test(test_client),
		cmocka_unit_test(test_server),
	};

	return cmocka_run_group_tests(tests, NULL, NULL);
}
