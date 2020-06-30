/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2020, Intel Corporation
 */

/*
 * integration-test.c -- connection integration tests
 */

#include <rdma/rdma_cma.h>
#include <stdlib.h>

#include "cmocka_headers.h"
#include "conn_req.h"
#include "integration-test.h"
#include "librpma.h"
#include "out.h"

#define MOCK_ADDR	"127.0.0.1"
#define MOCK_SERVICE	"1234"
#define MOCK_VERBS	((struct ibv_context *)0xABC1)
#define MOCK_VALIDATE	1
#define MOCK_CQ		(struct ibv_cq *)0x00C0
#define MOCK_EVCH	(struct rdma_event_channel *)0xE4C4
#define MOCK_OK		0
#define MOCK_SRC_ADDR	(struct sockaddr *)0x0ADD
#define MOCK_DST_ADDR	(struct sockaddr *)0x0ADE
#define MOCK_IBV_PD	(struct ibv_pd *)0x00D0

/* mocks */

/*
 * rdma_create_id -- mock of rdma_create_id
 */
int
rdma_create_id(struct rdma_event_channel *channel,
		struct rdma_cm_id **id, void *context,
		enum rdma_port_space ps)
{
	assert_non_null(id);
	assert_null(context);
	assert_int_equal(ps, RDMA_PS_TCP);

	/* allocate (struct rdma_cm_id *) */
	*id = mock_type(struct rdma_cm_id *);
	if (*id == NULL) {
		errno = mock_type(int);
		return -1;
	}

	return 0;
}

/*
 * rdma_destroy_id -- mock of rdma_destroy_id
 */
int
rdma_destroy_id(struct rdma_cm_id *id)
{
	errno = mock_type(int);
	if (errno)
		return -1;

	return 0;
}

struct ibv_alloc_pd_mock_args {
	int validate_params;
	struct ibv_pd *pd;
};

/*
 * ibv_alloc_pd -- ibv_alloc_pd() mock
 */
struct ibv_pd *
ibv_alloc_pd(struct ibv_context *ibv_ctx)
{
	struct ibv_alloc_pd_mock_args *args =
			mock_type(struct ibv_alloc_pd_mock_args *);
	if (args->validate_params == MOCK_VALIDATE)
		check_expected_ptr(ibv_ctx);

	if (args->pd != NULL)
		return args->pd;

	/*
	 * The ibv_alloc_pd(3) manual page does not document that this function
	 * returns any error via errno but seemingly it is. For the usability
	 * sake, in librpma we try to deduce what really happened using the
	 * errno value.
	 */
	errno = mock_type(int);

	return NULL;
}

/*
 * rdma_resolve_route -- mock of rdma_resolve_route
 */
int
rdma_resolve_route(struct rdma_cm_id *id, int timeout_ms)
{
	check_expected(id);
	assert_int_equal(timeout_ms, RPMA_DEFAULT_TIMEOUT);

	errno = mock_type(int);
	if (errno)
		return -1;

	return 0;
}

/*
 * ibv_create_cq -- ibv_create_cq() mock
 */
struct ibv_cq *
ibv_create_cq(struct ibv_context *context, int cqe, void *cq_context,
		struct ibv_comp_channel *channel, int comp_vector)
{
	assert_ptr_equal(context, MOCK_VERBS);
	assert_int_equal(cqe, RPMA_DEFAULT_Q_SIZE);
	assert_null(channel);
	assert_int_equal(comp_vector, 0);

	struct ibv_cq *cq = mock_type(struct ibv_cq *);
	if (!cq) {
		errno = mock_type(int);
		return NULL;
	}

	return cq;
}

/*
 * rdma_accept -- rdma_accept() mock
 */
int
rdma_accept(struct rdma_cm_id *id, struct rdma_conn_param *conn_param)
{
	check_expected(id);
	assert_non_null(conn_param);
	assert_null(conn_param->private_data);
	assert_int_equal(conn_param->private_data_len, 0);
	assert_int_equal(conn_param->responder_resources, RDMA_MAX_RESP_RES);
	assert_int_equal(conn_param->initiator_depth, RDMA_MAX_INIT_DEPTH);
	assert_int_equal(conn_param->flow_control, 1);
	assert_int_equal(conn_param->retry_count, 7); /* max 3-bit value */
	assert_int_equal(conn_param->rnr_retry_count, 7); /* max 3-bit value */

	errno = mock_type(int);
	if (errno)
		return -1;

	return 0;
}

/*
 * rdma_ack_cm_event -- rdma_ack_cm_event() mock
 */
int
rdma_ack_cm_event(struct rdma_cm_event *event)
{
	errno = mock_type(int);
	if (errno)
		return -1;

	return 0;
}

/*
 * rdma_get_cm_event -- rdma_get_cm_event() mock
 */
int
rdma_get_cm_event(struct rdma_event_channel *channel,
		struct rdma_cm_event **event_ptr)
{
	check_expected_ptr(channel);
	assert_non_null(event_ptr);

	struct rdma_cm_event *event = mock_type(struct rdma_cm_event *);
	if (!event) {
		errno = mock_type(int);
		return -1;
	}

	*event_ptr = event;
	return 0;
}

/*
 * rdma_disconnect -- rdma_disconnect() mock
 */
int
rdma_disconnect(struct rdma_cm_id *id)
{
	check_expected_ptr(id);
	errno = mock_type(int);
	if (errno)
		return -1;

	return 0;
}

/*
 * rdma_destroy_qp -- rdma_destroy_qp() mock
 */
void
rdma_destroy_qp(struct rdma_cm_id *id)
{
	check_expected_ptr(id);
}

/*
 * ibv_destroy_cq -- ibv_destroy_cq() mock
 */
int
ibv_destroy_cq(struct ibv_cq *cq)
{
	return mock_type(int);
}

struct ibv_dealloc_pd_mock_args {
	int validate_params;
	int ret;
};

/*
 * ibv_dealloc_pd -- ibv_dealloc_pd() mock
 */
int
ibv_dealloc_pd(struct ibv_pd *pd)
{
	struct ibv_dealloc_pd_mock_args *args =
			mock_type(struct ibv_dealloc_pd_mock_args *);
	if (args->validate_params == MOCK_VALIDATE)
		check_expected_ptr(pd);

	return args->ret;
}

/*
 * rdma_create_event_channel -- rdma_create_event_channel() mock
 */
struct rdma_event_channel *
rdma_create_event_channel(void)
{
	struct rdma_event_channel *evch =
		mock_type(struct rdma_event_channel *);
	if (!evch) {
		errno = mock_type(int);
		return NULL;
	}

	return evch;
}

static int Rdma_migrate_id_counter = 0;
#define RDMA_MIGRATE_TO_EVCH 0
#define RDMA_MIGRATE_FROM_EVCH 1

/*
 * rdma_migrate_id -- rdma_migrate_id() mock
 */
int
rdma_migrate_id(struct rdma_cm_id *id, struct rdma_event_channel *channel)
{
	/*
	 * This mock assumes the first call to rdma_migrate_id() always migrate
	 * a CM ID to an event channel. Whereas the second call migrate the
	 * CM ID from the event channel (channel == NULL).
	 */
	if (Rdma_migrate_id_counter == RDMA_MIGRATE_TO_EVCH)
		assert_ptr_equal(channel, MOCK_EVCH);
	else if (Rdma_migrate_id_counter == RDMA_MIGRATE_FROM_EVCH)
		assert_ptr_equal(channel, NULL);
	else
		assert_true(0);

	++Rdma_migrate_id_counter;

	errno = mock_type(int);
	if (errno)
		return -1;

	return 0;
}

/*
 * rdma_destroy_event_channel -- rdma_destroy_event_channel() mock
 */
void
rdma_destroy_event_channel(struct rdma_event_channel *channel)
{
	check_expected_ptr(channel);
}

/*
 * rdma_connect -- rdma_connect() mock
 */
int
rdma_connect(struct rdma_cm_id *id, struct rdma_conn_param *conn_param)
{
	check_expected(id);
	assert_non_null(conn_param);
	assert_int_equal(conn_param->responder_resources, RDMA_MAX_RESP_RES);
	assert_int_equal(conn_param->initiator_depth, RDMA_MAX_INIT_DEPTH);
	assert_int_equal(conn_param->flow_control, 1);
	assert_int_equal(conn_param->retry_count, 7); /* max 3-bit value */
	assert_int_equal(conn_param->rnr_retry_count, 7); /* max 3-bit value */

	errno = mock_type(int);
	if (errno)
		return -1;

	return 0;
}

/*
 * rdma_reject -- rdma_reject() mock
 */
int
rdma_reject(struct rdma_cm_id *id, const void *private_data,
		uint8_t private_data_len)
{
	check_expected_ptr(id);
	assert_null(private_data);
	assert_int_equal(private_data_len, 0);

	errno = mock_type(int);
	if (errno)
		return -1;

	return 0;
}

/*
 * rdma_listen -- rdma_listen() mock
 */
int
rdma_listen(struct rdma_cm_id *id, int backlog)
{
	check_expected_ptr(id);
	assert_int_equal(backlog, 0);

	errno = mock_type(int);
	if (errno)
		return -1;

	return 0;
}

struct rdma_addrinfo_args {
	int validate_params;
	struct rdma_addrinfo *res;
};

/*
 * rdma_getaddrinfo -- rdma_getaddrinfo() mock
 */
int
rdma_getaddrinfo(const char *node, const char *service,
		const struct rdma_addrinfo *hints, struct rdma_addrinfo **res)
{
	struct rdma_addrinfo_args *args =
				mock_type(struct rdma_addrinfo_args *);
	if (args->validate_params == MOCK_VALIDATE) {
		assert_string_equal(node, MOCK_ADDR);
		check_expected(hints->ai_flags);
	}

	*res = args->res;

	if (*res != NULL)
		return 0;

	errno = mock_type(int);

	return -1;
}

/*
 * rdma_freeaddrinfo -- rdma_freeaddrinfo() mock
 */
void
rdma_freeaddrinfo(struct rdma_addrinfo *res)
{
	struct rdma_addrinfo_args *args =
				mock_type(struct rdma_addrinfo_args *);
	if (args->validate_params == MOCK_VALIDATE)
		assert_ptr_equal(res, args->res);
}

/*
 * rdma_resolve_addr -- rdma_resolve_addr() mock
 * Note: CM ID is not modified.
 */
int
rdma_resolve_addr(struct rdma_cm_id *id, struct sockaddr *src_addr,
		struct sockaddr *dst_addr, int timeout_ms)
{
	check_expected_ptr(id);
	check_expected_ptr(src_addr);
	check_expected_ptr(dst_addr);
	check_expected(timeout_ms);

	errno = mock_type(int);
	if (errno)
		return -1;

	return 0;
}

/*
 * rdma_bind_addr -- rdma_bind_addr() mock
 * Note: CM ID is not modified.
 */
int
rdma_bind_addr(struct rdma_cm_id *id, struct sockaddr *addr)
{
	check_expected_ptr(id);
	check_expected_ptr(addr);

	errno = mock_type(int);
	if (errno)
		return -1;

	return 0;
}

/*
 * rdma_create_qp -- rdma_create_qp() mock
 */
int
rdma_create_qp(struct rdma_cm_id *id, struct ibv_pd *pd,
		struct ibv_qp_init_attr *qp_init_attr)
{
	errno = mock_type(int);
	if (errno)
		return -1;

	return 0;
}

/*
 * ibv_reg_mr -- ibv_reg_mr() mock
 */
struct ibv_mr *
ibv_reg_mr(struct ibv_pd *pd, void *addr, size_t length, int access)
{
	check_expected_ptr(pd);
	check_expected_ptr(addr);
	check_expected(length);
	check_expected(access);

	struct ibv_mr *mr = mock_type(struct ibv_mr *);
	if (mr == NULL) {
		errno = mock_type(int);
		return NULL;
	}

	return mr;
}

void *__real__test_malloc(size_t size);

/*
 * __wrap__test_malloc -- malloc() mock
 */
void *
__wrap__test_malloc(size_t size)
{
	int err = mock_type(int);

	if (err) {
		errno = err;
		return NULL;
	}

	return __real__test_malloc(size);
}

struct info_state {
	struct rdma_addrinfo rai;
	struct rpma_info *info;
};

void
connection_client(void **unused)
{
	/* configure mocks for rpma_utils_get_ibv_context */
	static struct info_state istate;
	memset(&istate, 0, sizeof(istate));
	istate.rai.ai_src_addr = MOCK_SRC_ADDR;
	istate.rai.ai_dst_addr = MOCK_DST_ADDR;
	struct rdma_addrinfo_args args = {MOCK_VALIDATE, &istate.rai};

	will_return(rdma_getaddrinfo, &args);
	expect_value(rdma_getaddrinfo, hints->ai_flags, 0);
	will_return(__wrap__test_malloc, MOCK_OK);

	struct rdma_cm_id id;
	id.verbs = MOCK_VERBS;
	will_return(rdma_create_id, &id);

	expect_value(rdma_resolve_addr, id, &id);
	expect_value(rdma_resolve_addr, src_addr, MOCK_SRC_ADDR);
	expect_value(rdma_resolve_addr, dst_addr, MOCK_DST_ADDR);
	expect_value(rdma_resolve_addr, timeout_ms, RPMA_DEFAULT_TIMEOUT);
	will_return(rdma_resolve_addr, MOCK_OK);

	will_return(rdma_destroy_id, 0);

	will_return(rdma_freeaddrinfo, &args);

	/* configure mocks for rpma_peer_new */
	struct ibv_alloc_pd_mock_args alloc_args = {MOCK_VALIDATE, MOCK_IBV_PD};
	will_return(ibv_alloc_pd, &alloc_args);
	expect_value(ibv_alloc_pd, ibv_ctx, MOCK_VERBS);
	will_return(__wrap__test_malloc, MOCK_OK);

	/* configure mocks for rpma_conn_req_new() */
	will_return(rdma_getaddrinfo, &args);
	expect_value(rdma_getaddrinfo, hints->ai_flags, 0);
	will_return(__wrap__test_malloc, MOCK_OK);

	will_return(rdma_create_id, &id);

	expect_value(rdma_resolve_addr, id, &id);
	expect_value(rdma_resolve_addr, src_addr, MOCK_SRC_ADDR);
	expect_value(rdma_resolve_addr, dst_addr, MOCK_DST_ADDR);
	expect_value(rdma_resolve_addr, timeout_ms, RPMA_DEFAULT_TIMEOUT);
	will_return(rdma_resolve_addr, MOCK_OK);

	expect_value(rdma_resolve_route, id, &id);
	will_return(rdma_resolve_route, MOCK_OK);

	will_return(ibv_create_cq, MOCK_CQ);

	will_return(rdma_create_qp, MOCK_OK);
	will_return(__wrap__test_malloc, MOCK_OK);

	will_return(rdma_freeaddrinfo, &args);

	/* configure mocks for rpma_conn_req_connect() */
	will_return(rdma_create_event_channel, MOCK_EVCH);

	will_return(rdma_migrate_id, MOCK_OK);
	will_return_maybe(__wrap__test_malloc, MOCK_OK);

	expect_value(rdma_connect, id, &id);
	will_return(rdma_connect, MOCK_OK);

	will_return(rdma_ack_cm_event, MOCK_OK);

	/* configure mocks for rpma_conn_next_event() */
	expect_value(rdma_get_cm_event, channel, MOCK_EVCH);

	struct rdma_cm_event f_event;
	const char *msg = "Hello server!";
	f_event.event = RDMA_CM_EVENT_ESTABLISHED;
	f_event.param.conn.private_data = (void *)msg;
	f_event.param.conn.private_data_len = (strlen(msg) + 1) * sizeof(char);

	will_return(rdma_get_cm_event, &f_event);

	will_return(rdma_ack_cm_event, MOCK_OK);

	/* configure mocks for rpma_conn_next_event() */
	struct rdma_cm_event s_event;
	s_event.event = RDMA_CM_EVENT_DISCONNECTED;
	expect_value(rdma_get_cm_event, channel, MOCK_EVCH);
	will_return(rdma_get_cm_event, &s_event);

	/* configure mocks for rpma_conn_disconnect() */
	expect_value(rdma_disconnect, id, &id);
	will_return(rdma_disconnect, MOCK_OK);

	/* configure mocks for rpma_conn_delete() */
	expect_value(rdma_destroy_qp, id, &id);

	will_return(ibv_destroy_cq, MOCK_OK);

	will_return(rdma_destroy_id, MOCK_OK);

	expect_value(rdma_destroy_event_channel, channel, MOCK_EVCH);

	/* configure mocks for rpma_peer_delete() */
	struct ibv_dealloc_pd_mock_args dealloc_args =
		{MOCK_VALIDATE, MOCK_OK};
	will_return(ibv_dealloc_pd, &dealloc_args);
	expect_value(ibv_dealloc_pd, pd, MOCK_IBV_PD);

	int ret = client_main(MOCK_ADDR, MOCK_SERVICE);

	assert_int_equal(ret, MOCK_OK);
}

int
main(int argc, char *argv[])
{
	const struct CMUnitTest tests[] = {
		cmocka_unit_test(connection_client),
	};

	return cmocka_run_group_tests(tests, NULL, NULL);
}
