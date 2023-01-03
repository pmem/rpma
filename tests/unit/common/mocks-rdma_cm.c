// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020-2022, Intel Corporation */
/* Copyright (c) 2021-2022, Fujitsu Limited */

/*
 * mocks-rdma_cm.c -- librdmacm mocks
 */

#include <rdma/rdma_cma.h>
#include <librpma.h>

#include "cmocka_headers.h"
#include "conn_req.h"
#include "mocks-ibverbs.h"
#include "mocks-rdma_cm.h"
#include "test-common.h"

struct rdma_event_channel Evch;		/* mock event channel */
struct rdma_cm_id Cm_id;		/* mock CM ID */
struct ibv_sa_path_rec Path_rec;	/* mock ibv_sa_path_rec */

/*
 * Rdma_migrate_id_counter -- counter of calls to rdma_migrate_id() which allows
 * controlling its mock behaviour from call-to-call.
 */
int Rdma_migrate_id_counter = 0;

/* mock control entity */
int Mock_ctrl_defer_destruction = MOCK_CTRL_NO_DEFER;

const struct rdma_cm_id Cmid_zero = {0};

/*
 * rdma_create_qp_ex -- rdma_create_qp_ex() mock
 */
int
rdma_create_qp_ex(struct rdma_cm_id *id, struct ibv_qp_init_attr_ex *qp_init_attr)
{
	check_expected_ptr(id);
	assert_non_null(qp_init_attr);
	check_expected(qp_init_attr->qp_context);
	check_expected(qp_init_attr->send_cq);
	check_expected(qp_init_attr->recv_cq);
	check_expected(qp_init_attr->srq);
	check_expected(qp_init_attr->cap.max_send_wr);
	check_expected(qp_init_attr->cap.max_recv_wr);
	check_expected(qp_init_attr->cap.max_send_sge);
	check_expected(qp_init_attr->cap.max_recv_sge);
	check_expected(qp_init_attr->cap.max_inline_data);
	assert_int_equal(qp_init_attr->qp_type, IBV_QPT_RC);
	assert_int_equal(qp_init_attr->sq_sig_all, 0);
	check_expected(qp_init_attr->comp_mask);
#ifdef IBV_WR_ATOMIC_WRITE_SUPPORTED
	if (qp_init_attr->comp_mask & IBV_QP_INIT_ATTR_SEND_OPS_FLAGS)
		check_expected(qp_init_attr->send_ops_flags);
#endif
	check_expected(qp_init_attr->pd);

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

	if (!Mock_ctrl_defer_destruction)
		expect_value(rdma_destroy_id, id, *id);

	return 0;
}

/*
 * rdma_destroy_id -- mock of rdma_destroy_id
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
 * rdma_resolve_route -- mock of rdma_resolve_route
 */
int
rdma_resolve_route(struct rdma_cm_id *id, int timeout_ms)
{
	check_expected(id);
	check_expected(timeout_ms);

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
	assert_ptr_equal(channel, MOCK_EVCH);
}

/*
 * rdma_migrate_id -- rdma_migrate_id() mock
 */
int
rdma_migrate_id(struct rdma_cm_id *id, struct rdma_event_channel *channel)
{
	assert_ptr_equal(id, MOCK_CM_ID);

	/*
	 * This mock assumes the first call to rdma_migrate_id() always migrate
	 * a CM ID to an event channel. Whereas the second call migrate
	 * the CM ID from the event channel (channel == NULL).
	 */
	if (Rdma_migrate_id_counter == RDMA_MIGRATE_TO_EVCH)
		assert_ptr_equal(channel, MOCK_EVCH);
	else if (Rdma_migrate_id_counter == RDMA_MIGRATE_FROM_EVCH)
		assert_ptr_equal(channel, NULL);
	else
		assert_true(0);

	++Rdma_migrate_id_counter;
	id->qp = MOCK_QP;

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
 * rdma_getaddrinfo -- rdma_getaddrinfo() mock
 */
#ifdef RDMA_GETADDRINFO_OLD_SIGNATURE
int
rdma_getaddrinfo(char *node, char *port,
		struct rdma_addrinfo *hints, struct rdma_addrinfo **res)
#else
int
rdma_getaddrinfo(const char *node, const char *port,
		const struct rdma_addrinfo *hints, struct rdma_addrinfo **res)
#endif
{
	struct rdma_addrinfo_args *args =
				mock_type(struct rdma_addrinfo_args *);
	if (args->validate_params == MOCK_VALIDATE) {
		assert_string_equal(node, MOCK_IP_ADDRESS);
		assert_string_equal(port, MOCK_PORT);
		check_expected(hints->ai_flags);
	}

	*res = args->res;

	if (*res != NULL)
		return 0;

	int ret = mock_type(int);
	assert_int_not_equal(ret, 0);
	errno = mock_type(int);

	return ret;
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
 * rdma_event_str -- rdma_event_str() mock
 */
const char *
rdma_event_str(enum rdma_cm_event_type event)
{
	return "";
}
