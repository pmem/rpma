// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020-2021, Intel Corporation */

/*
 * example-01-connection.c -- connection integration tests
 */

#include <rdma/rdma_cma.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "cmocka_headers.h"
#include "librpma.h"
#include "mocks.h"

int client_main(int argc, char *argv[]);
int server_main(int argc, char *argv[]);

/* tests */

/*
 * test_client__success -- connection integration test, client
 */
void
test_client__success(void **unused)
{
	/* configure mocks for rpma_utils_get_ibv_context */
	struct rdma_addrinfo res1;
	will_return(rdma_getaddrinfo, MOCK_OK);
	will_return(rdma_getaddrinfo, &res1);
	will_return_always(__wrap__test_malloc, MOCK_OK);

	struct rdma_cm_id id;
	will_return(rdma_create_id, &id);

	expect_value(rdma_resolve_addr, id, &id);
	expect_value(rdma_resolve_addr, src_addr, NULL);
	expect_value(rdma_resolve_addr, dst_addr, NULL);
	expect_value(rdma_resolve_addr, timeout_ms, MOCK_TIMEOUT);
	will_return(rdma_resolve_addr, MOCK_VERBS);
	will_return(rdma_resolve_addr, MOCK_OK);

	expect_value(rdma_destroy_id, id, &id);
	will_return(rdma_destroy_id, MOCK_OK);

	expect_value(rdma_freeaddrinfo, res, &res1);

	/* configure mocks for rpma_peer_new */
#ifdef ON_DEMAND_PAGING_SUPPORTED
	will_return(ibv_query_device_ex_mock, &Ibv_odp_capable_caps);
#endif
	expect_value(ibv_alloc_pd, ibv_ctx, MOCK_VERBS);
	will_return(ibv_alloc_pd, MOCK_IBV_PD);

	/* configure mocks for rpma_conn_req_new() */
	struct rdma_addrinfo res2;
	will_return(rdma_getaddrinfo, MOCK_OK);
	will_return(rdma_getaddrinfo, &res2);
	will_return(rdma_getaddrinfo, MOCK_SRC_ADDR);
	will_return(rdma_getaddrinfo, MOCK_DST_ADDR);

	will_return(rdma_create_id, &id);

	expect_value(rdma_resolve_addr, id, &id);
	expect_value(rdma_resolve_addr, src_addr, MOCK_SRC_ADDR);
	expect_value(rdma_resolve_addr, dst_addr, MOCK_DST_ADDR);
	expect_value(rdma_resolve_addr, timeout_ms, MOCK_TIMEOUT);
	will_return(rdma_resolve_addr, MOCK_VERBS);
	will_return(rdma_resolve_addr, MOCK_OK);

	expect_value(rdma_resolve_route, id, &id);
	will_return(rdma_resolve_route, MOCK_OK);

	will_return(ibv_create_comp_channel, &Ibv_comp_channel);

	will_return(ibv_create_cq, MOCK_CQ);

	will_return(ibv_req_notify_cq_mock, MOCK_OK);

	expect_value(rdma_create_qp, id, &id);
	expect_value(rdma_create_qp, pd, MOCK_IBV_PD);
	will_return(rdma_create_qp, MOCK_OK);
	will_return(rdma_create_qp, MOCK_QP);

	expect_value(rdma_freeaddrinfo, res, &res2);

	/* configure mocks for rpma_conn_req_connect() */
	will_return(rdma_create_event_channel, MOCK_EVCH);

	expect_value(rdma_migrate_id, id, &id);
	expect_value(rdma_migrate_id, channel, MOCK_EVCH);
	will_return(rdma_migrate_id, MOCK_OK);

	struct mmap_args allocated_raw = {0};
	will_return(__wrap_mmap, MOCK_OK);
	will_return(__wrap_mmap, &allocated_raw);

	expect_value(ibv_reg_mr, pd, MOCK_IBV_PD);
	expect_value(ibv_reg_mr, length, MOCK_RAW_SIZE);
	expect_value(ibv_reg_mr, access, IBV_ACCESS_LOCAL_WRITE);
	will_return(ibv_reg_mr, &allocated_raw.addr);
	will_return(ibv_reg_mr, MOCK_MR);

	expect_value(rdma_connect, id, &id);
	will_return(rdma_connect, MOCK_OK);

	/* configure mocks for rpma_conn_next_event() */
	struct rdma_cm_event f_event;
	const char *msg = "Hello client!";
	f_event.event = RDMA_CM_EVENT_ESTABLISHED;
	f_event.param.conn.private_data = (void *)msg;
	f_event.param.conn.private_data_len = (strlen(msg) + 1) * sizeof(char);

	expect_value(rdma_get_cm_event, channel, MOCK_EVCH);
	will_return(rdma_get_cm_event, &f_event);

	expect_value(rdma_ack_cm_event, event, &f_event);
	will_return(rdma_ack_cm_event, MOCK_OK);

	/* configure mocks for rpma_conn_next_event() */
	struct rdma_cm_event s_event;
	s_event.event = RDMA_CM_EVENT_DISCONNECTED;
	expect_value(rdma_get_cm_event, channel, MOCK_EVCH);
	will_return(rdma_get_cm_event, &s_event);

	expect_value(rdma_ack_cm_event, event, &s_event);
	will_return(rdma_ack_cm_event, MOCK_OK);

	/* configure mocks for rpma_conn_disconnect() */
	expect_value(rdma_disconnect, id, &id);
	will_return(rdma_disconnect, MOCK_OK);

	/* configure mocks for rpma_conn_delete() */
	expect_value(ibv_dereg_mr, mr, MOCK_MR);
	will_return(ibv_dereg_mr, MOCK_OK);
	will_return(__wrap_munmap, &allocated_raw);
	will_return(__wrap_munmap, MOCK_OK);

	expect_value(rdma_destroy_qp, id, &id);

	expect_value(ibv_destroy_cq, cq, MOCK_CQ);
	will_return(ibv_destroy_cq, MOCK_OK);

	will_return(ibv_destroy_comp_channel, MOCK_OK);

	expect_value(rdma_destroy_id, id, &id);
	will_return(rdma_destroy_id, MOCK_OK);

	expect_value(rdma_destroy_event_channel, channel, MOCK_EVCH);

	/* configure mocks for rpma_peer_delete() */
	expect_value(ibv_dealloc_pd, pd, MOCK_IBV_PD);
	will_return(ibv_dealloc_pd, MOCK_OK);

	/* run test */
	char *argv[] = {"client", MOCK_ADDR, MOCK_PORT};
	int ret = client_main(3, argv);

	assert_int_equal(ret, MOCK_OK);
}

/*
 * test_server__success -- connection integration test, server
 */
void
test_server__success(void **unused)
{
	/* configure mocks for rpma_utils_get_ibv_context */
	struct rdma_addrinfo res1;
	will_return(rdma_getaddrinfo, MOCK_OK);
	will_return(rdma_getaddrinfo, &res1);
	will_return_always(__wrap__test_malloc, MOCK_OK);

	struct rdma_cm_id id;
	will_return(rdma_create_id, &id);

	expect_value(rdma_bind_addr, id, &id);
	expect_value(rdma_bind_addr, addr, NULL);
	will_return(rdma_bind_addr, MOCK_VERBS);
	will_return(rdma_bind_addr, MOCK_OK);

	expect_value(rdma_destroy_id, id, &id);
	will_return(rdma_destroy_id, MOCK_OK);

	expect_value(rdma_freeaddrinfo, res, &res1);

	/* configure mocks for rpma_peer_new */
#ifdef ON_DEMAND_PAGING_SUPPORTED
	will_return(ibv_query_device_ex_mock, &Ibv_odp_capable_caps);
#endif
	expect_value(ibv_alloc_pd, ibv_ctx, MOCK_VERBS);
	will_return(ibv_alloc_pd, MOCK_IBV_PD);

	/* configure mocks for rpma_ep_listen() */
	will_return(rdma_create_event_channel, MOCK_EVCH);

	will_return(rdma_create_id, &id);

	struct rdma_addrinfo res2;
	will_return(rdma_getaddrinfo, MOCK_OK);
	will_return(rdma_getaddrinfo, &res2);
	will_return(rdma_getaddrinfo, MOCK_SRC_ADDR);
	will_return(rdma_getaddrinfo, MOCK_DST_ADDR);

	expect_value(rdma_bind_addr, id, &id);
	expect_value(rdma_bind_addr, addr, MOCK_SRC_ADDR);
	will_return(rdma_bind_addr, MOCK_VERBS);
	will_return(rdma_bind_addr, MOCK_OK);

	expect_value(rdma_listen, id, &id);
	will_return(rdma_listen, MOCK_OK);

	expect_value(rdma_freeaddrinfo, res, &res2);

	/* configure mocks for rpma_ep_next_conn_req() */
	struct rdma_cm_event f_event;
	const char *msg = "Hello client!";
	f_event.param.conn.private_data = NULL;
	f_event.param.conn.private_data_len = (strlen(msg) + 1) * sizeof(char);
	f_event.event = RDMA_CM_EVENT_CONNECT_REQUEST;
	f_event.id = &id;
	expect_value(rdma_get_cm_event, channel, MOCK_EVCH);
	will_return(rdma_get_cm_event, &f_event);

	will_return(ibv_create_comp_channel, &Ibv_comp_channel);

	will_return(ibv_create_cq, MOCK_CQ);

	will_return(ibv_req_notify_cq_mock, MOCK_OK);

	expect_value(rdma_create_qp, id, &id);
	expect_value(rdma_create_qp, pd, MOCK_IBV_PD);
	will_return(rdma_create_qp, MOCK_OK);
	will_return(rdma_create_qp, MOCK_QP);

	/* configure mocks for rpma_conn_req_connect() */
	expect_value(rdma_accept, id, &id);
	will_return(rdma_accept, MOCK_OK);

	expect_value(rdma_ack_cm_event, event, &f_event);
	will_return(rdma_ack_cm_event, MOCK_OK);

	will_return(rdma_create_event_channel, MOCK_EVCH);

	expect_value(rdma_migrate_id, id, &id);
	expect_value(rdma_migrate_id, channel, MOCK_EVCH);
	will_return(rdma_migrate_id, MOCK_OK);

	struct mmap_args allocated_raw = {0};
	will_return(__wrap_mmap, MOCK_OK);
	will_return(__wrap_mmap, &allocated_raw);

	expect_value(ibv_reg_mr, pd, MOCK_IBV_PD);
	expect_value(ibv_reg_mr, length, MOCK_RAW_SIZE);
	expect_value(ibv_reg_mr, access, IBV_ACCESS_LOCAL_WRITE);
	will_return(ibv_reg_mr, &allocated_raw.addr);
	will_return(ibv_reg_mr, MOCK_MR);

	/* configure mocks for rpma_conn_next_event() */
	struct rdma_cm_event s_event;
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
	expect_value(rdma_disconnect, id, &id);
	will_return(rdma_disconnect, MOCK_OK);

	/* configure mocks for rpma_conn_next_event() */
	struct rdma_cm_event t_event;
	t_event.event = RDMA_CM_EVENT_DISCONNECTED;
	expect_value(rdma_get_cm_event, channel, MOCK_EVCH);
	will_return(rdma_get_cm_event, &t_event);

	expect_value(rdma_ack_cm_event, event, &t_event);
	will_return(rdma_ack_cm_event, MOCK_OK);

	/* configure mocks for rpma_conn_delete() */
	expect_value(ibv_dereg_mr, mr, MOCK_MR);
	will_return(ibv_dereg_mr, MOCK_OK);
	will_return(__wrap_munmap, &allocated_raw);
	will_return(__wrap_munmap, MOCK_OK);

	expect_value(rdma_destroy_qp, id, &id);

	expect_value(ibv_destroy_cq, cq, MOCK_CQ);
	will_return(ibv_destroy_cq, MOCK_OK);

	will_return(ibv_destroy_comp_channel, MOCK_OK);

	expect_value(rdma_destroy_id, id, &id);
	will_return(rdma_destroy_id, MOCK_OK);

	expect_value(rdma_destroy_event_channel, channel, MOCK_EVCH);

	/* configure mocks for rpma_ep_shutdown() */
	expect_value(rdma_destroy_id, id, &id);
	will_return(rdma_destroy_id, MOCK_OK);

	expect_value(rdma_destroy_event_channel, channel, MOCK_EVCH);

	/* configure mocks for rpma_peer_delete() */
	expect_value(ibv_dealloc_pd, pd, MOCK_IBV_PD);
	will_return(ibv_dealloc_pd, MOCK_OK);

	/* run test */
	char *argv[] = {"server", MOCK_ADDR, MOCK_PORT};
	int ret = server_main(3, argv);

	assert_int_equal(ret, MOCK_OK);
}

int
main(int argc, char *argv[])
{
	MOCK_VERBS->abi_compat = __VERBS_ABI_IS_EXTENDED;
#ifdef ON_DEMAND_PAGING_SUPPORTED
	Verbs_context.query_device_ex = ibv_query_device_ex_mock;
#endif
	Verbs_context.sz = sizeof(struct verbs_context);
	MOCK_VERBS->ops.req_notify_cq = ibv_req_notify_cq_mock;
	Ibv_cq.context = MOCK_VERBS;

	const struct CMUnitTest tests[] = {
		cmocka_unit_test(test_client__success),
		cmocka_unit_test(test_server__success),
	};

	return cmocka_run_group_tests(tests, NULL, NULL);
}
