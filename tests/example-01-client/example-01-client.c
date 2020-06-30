/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2020, Intel Corporation
 */

/*
 * example-01-client.c -- connection integration tests
 */

#include <rdma/rdma_cma.h>
#include <stdlib.h>
#include <stdio.h>

#include "cmocka_headers.h"
#include "librpma.h"

#define MOCK_ADDR		"127.0.0.1"
#define MOCK_SERVICE		"1234"
#define MOCK_VERBS		((struct ibv_context *)0xABC1)
#define MOCK_CQ			(struct ibv_cq *)0x00C0
#define MOCK_EVCH		(struct rdma_event_channel *)0xE4C4
#define MOCK_SRC_ADDR		(struct sockaddr *)0x0ADD
#define MOCK_DST_ADDR		(struct sockaddr *)0x0ADE
#define MOCK_IBV_PD		(struct ibv_pd *)0x00D0
#define MOCK_TIMEOUT		1000 /* RPMA_DEFAULT_TIMEOUT */
#define MOCK_DEFAULT_Q_SIZE	10 /* RPMA_DEFAULT_Q_SIZE */
#define MOCK_MAX_SGE		1 /* RPMA_MAX_SGE */
#define MOCK_MAX_INLINE_DATA	0 /* RPMA_MAX_INLINE_DATA */
#define MOCK_OK			0

/* mocks */

/*
 * rdma_create_id -- rdma_create_id() mock
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

	memset(*id, 0, sizeof(struct rdma_cm_id));

	return 0;
}

/*
 * rdma_destroy_id -- rdma_destroy_id() mock
 */
int
rdma_destroy_id(struct rdma_cm_id *id)
{
	check_expected(id);

	errno = mock_type(int);
	if (errno)
		return -1;

	return 0;
}

/*
 * ibv_alloc_pd -- ibv_alloc_pd() mock
 */
struct ibv_pd *
ibv_alloc_pd(struct ibv_context *ibv_ctx)
{
	check_expected_ptr(ibv_ctx);

	struct ibv_pd *pd = mock_type(struct ibv_pd *);

	if (pd != NULL)
		return pd;

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
 * ibv_dealloc_pd -- ibv_dealloc_pd() mock
 */
int
ibv_dealloc_pd(struct ibv_pd *pd)
{
	check_expected_ptr(pd);

	return mock_type(int);
}

/*
 * ibv_create_cq -- ibv_create_cq() mock
 */
struct ibv_cq *
ibv_create_cq(struct ibv_context *context, int cqe, void *cq_context,
		struct ibv_comp_channel *channel, int comp_vector)
{
	assert_ptr_equal(context, MOCK_VERBS);
	assert_int_equal(cqe, MOCK_DEFAULT_Q_SIZE);
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
 * ibv_destroy_cq -- ibv_destroy_cq() mock
 */
int
ibv_destroy_cq(struct ibv_cq *cq)
{
	check_expected_ptr(cq);

	return mock_type(int);
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
 * rdma_resolve_route -- rdma_resolve_route() mock
 */
int
rdma_resolve_route(struct rdma_cm_id *id, int timeout_ms)
{
	check_expected(id);
	assert_int_equal(timeout_ms, MOCK_TIMEOUT);

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
 * rdma_ack_cm_event -- rdma_ack_cm_event() mock
 */
int
rdma_ack_cm_event(struct rdma_cm_event *event)
{
	check_expected_ptr(event);

	errno = mock_type(int);
	if (errno)
		return -1;

	return 0;
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

/*
 * rdma_destroy_event_channel -- rdma_destroy_event_channel() mock
 */
void
rdma_destroy_event_channel(struct rdma_event_channel *channel)
{
	check_expected_ptr(channel);
}

/*
 * rdma_create_qp -- rdma_create_qp() mock
 */
int
rdma_create_qp(struct rdma_cm_id *id, struct ibv_pd *pd,
		struct ibv_qp_init_attr *qp_init_attr)
{
	check_expected_ptr(id);
	check_expected_ptr(pd);
	assert_non_null(qp_init_attr);
	assert_int_equal(qp_init_attr->qp_context, NULL);
	assert_int_equal(qp_init_attr->send_cq, MOCK_CQ);
	assert_int_equal(qp_init_attr->recv_cq, MOCK_CQ);
	assert_int_equal(qp_init_attr->srq, NULL);
	assert_int_equal(qp_init_attr->cap.max_send_wr, MOCK_DEFAULT_Q_SIZE);
	assert_int_equal(qp_init_attr->cap.max_recv_wr, MOCK_DEFAULT_Q_SIZE);
	assert_int_equal(qp_init_attr->cap.max_send_sge, MOCK_MAX_SGE);
	assert_int_equal(qp_init_attr->cap.max_recv_sge, MOCK_MAX_SGE);
	assert_int_equal(qp_init_attr->cap.max_inline_data,
		MOCK_MAX_INLINE_DATA);
	assert_int_equal(qp_init_attr->qp_type, IBV_QPT_RC);
	assert_int_equal(qp_init_attr->sq_sig_all, 0);

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

#if defined(ibv_reg_mr)
/*
 * Since rdma-core v27.0-105-g5a750676
 * ibv_reg_mr() has been defined as a macro
 * in <infiniband/verbs.h>:
 *
 * https://github.com/linux-rdma/rdma-core/commit/5a750676e8312715100900c6336bbc98577e082b
 *
 * In order to mock the ibv_reg_mr() function
 * the `ibv_reg_mr` symbol has to be undefined first
 * and the additional ibv_reg_mr_iova2() function
 * has to be mocked, because it is called
 * by the 'ibv_reg_mr' macro.
 */
#undef ibv_reg_mr

/*
 * ibv_reg_mr_iova2 -- ibv_reg_mr_iova2() mock
 */
struct ibv_mr *
ibv_reg_mr_iova2(struct ibv_pd *pd, void *addr, size_t length,
			uint64_t iova, unsigned access)
{
	return ibv_reg_mr(pd, addr, length, (int)access);
}
#endif

/*
 * ibv_reg_mr -- ibv_reg_mr() mock
 */
struct ibv_mr *
ibv_reg_mr(struct ibv_pd *pd, void *addr, size_t length, int access)
{
	assert_true(0);
	return NULL;
}

/*
 * rdma_migrate_id -- rdma_migrate_id() mock
 */
int
rdma_migrate_id(struct rdma_cm_id *id, struct rdma_event_channel *channel)
{
	check_expected_ptr(id);
	check_expected_ptr(channel);

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
	assert_true(0);
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

/*
 * rdma_getaddrinfo -- rdma_getaddrinfo() mock
 */
int
rdma_getaddrinfo(const char *node, const char *service,
		const struct rdma_addrinfo *hints, struct rdma_addrinfo **res)
{
	assert_string_equal(node, MOCK_ADDR);

	errno = mock_type(int);
	if (errno)
		return -1;

	struct rdma_addrinfo *buf = mock_type(struct rdma_addrinfo *);
	if (service == NULL) {
		buf->ai_src_addr = NULL;
		buf->ai_dst_addr = NULL;
	} else {
		assert_string_equal(service, MOCK_SERVICE);
		buf->ai_src_addr = mock_type(struct sockaddr *);
		buf->ai_dst_addr = mock_type(struct sockaddr *);
	}

	*res = buf;
	return 0;
}

/*
 * rdma_freeaddrinfo -- rdma_freeaddrinfo() mock
 */
void
rdma_freeaddrinfo(struct rdma_addrinfo *res)
{
	check_expected_ptr(res);
}

/*
 * rdma_resolve_addr -- rdma_resolve_addr() mock
 */
int
rdma_resolve_addr(struct rdma_cm_id *id, struct sockaddr *src_addr,
		struct sockaddr *dst_addr, int timeout_ms)
{
	check_expected_ptr(id);
	check_expected_ptr(src_addr);
	check_expected_ptr(dst_addr);
	check_expected(timeout_ms);

	id->verbs = mock_type(struct ibv_context *);
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

/*
 * fprintf -- fprintf() mock
 */
int
__wrap_fprintf(FILE *__restrict __stream, const char *__restrict __format, ...)
{
	return 0;
}

int client_main(int argc, char *argv[]);

/* tests */

/*
 * test -- connection integration test, client
 */
void
test(void **unused)
{
	/* configure mocks for rpma_utils_get_ibv_context */
	struct rdma_addrinfo *res1;
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
	expect_value(ibv_alloc_pd, ibv_ctx, MOCK_VERBS);
	will_return(ibv_alloc_pd, MOCK_IBV_PD);

	/* configure mocks for rpma_conn_req_new() */
	struct rdma_addrinfo *res2;
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

	will_return(ibv_create_cq, MOCK_CQ);

	expect_value(rdma_create_qp, id, &id);
	expect_value(rdma_create_qp, pd, MOCK_IBV_PD);
	will_return(rdma_create_qp, MOCK_OK);

	expect_value(rdma_freeaddrinfo, res, &res2);

	/* configure mocks for rpma_conn_req_connect() */
	will_return(rdma_create_event_channel, MOCK_EVCH);

	expect_value(rdma_migrate_id, id, &id);
	expect_value(rdma_migrate_id, channel, MOCK_EVCH);
	will_return(rdma_migrate_id, MOCK_OK);

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
	expect_value(rdma_destroy_qp, id, &id);

	expect_value(ibv_destroy_cq, cq, MOCK_CQ);
	will_return(ibv_destroy_cq, MOCK_OK);

	expect_value(rdma_destroy_id, id, &id);
	will_return(rdma_destroy_id, MOCK_OK);

	expect_value(rdma_destroy_event_channel, channel, MOCK_EVCH);

	/* configure mocks for rpma_peer_delete() */
	expect_value(ibv_dealloc_pd, pd, MOCK_IBV_PD);
	will_return(ibv_dealloc_pd, MOCK_OK);

	/* run test */
	char *argv[] = {"client", MOCK_ADDR, MOCK_SERVICE};
	int ret = client_main(3, argv);

	assert_int_equal(ret, MOCK_OK);
}

int
main(int argc, char *argv[])
{
	const struct CMUnitTest tests[] = {
		cmocka_unit_test(test),
	};

	return cmocka_run_group_tests(tests, NULL, NULL);
}
