// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020-2021, Intel Corporation */
/* Copyright 2021, Fujitsu */

/*
 * conn-common.c -- the connection unit tests common functions
 */

#include <string.h>

#include "conn-common.h"
#include "mocks-ibverbs.h"
#include "mocks-rdma_cm.h"

const char Private_data[] = "Random data";
const char Private_data_2[] = "Another random data";

struct conn_test_state Conn_without_rcq = {
	.rcq = NULL
};

struct conn_test_state Conn_with_rcq = {
	.rcq = MOCK_RPMA_RCQ
};

/*
 * rpma_private_data_store -- rpma_private_data_store() mock
 */
int
rpma_private_data_store(struct rdma_cm_event *edata,
		struct rpma_conn_private_data *pdata)
{
	const LargestIntegralType allowed_events[] = {
			RDMA_CM_EVENT_CONNECT_REQUEST,
			RDMA_CM_EVENT_ESTABLISHED};
	assert_non_null(edata);
	assert_in_set(edata->event, allowed_events,
		sizeof(allowed_events) / sizeof(allowed_events[0]));
	assert_non_null(pdata);
	assert_null(pdata->ptr);
	assert_int_equal(pdata->len, 0);

	int ret = mock_type(int);
	if (ret)
		return ret;

	pdata->ptr = (void *)edata->param.conn.private_data;
	pdata->len = edata->param.conn.private_data_len;

	return 0;
}

/*
 * rpma_private_data_discard -- rpma_private_data_discard() mock
 */
void
rpma_private_data_discard(struct rpma_conn_private_data *pdata)
{
	assert_non_null(pdata);
	check_expected(pdata->ptr);
	check_expected(pdata->len);
	pdata->ptr = NULL;
	pdata->len = 0;
}

/*
 * setup__conn_new - prepare a valid rpma_conn object
 */
int
setup__conn_new(void **cstate_ptr)
{
	/* the default is Conn_without_rcq */
	struct conn_test_state *cstate = *cstate_ptr ? *cstate_ptr :
			&Conn_without_rcq;
	cstate->conn = NULL;
	cstate->data.ptr = NULL;
	cstate->data.len = 0;

	Ibv_cq.channel = MOCK_COMP_CHANNEL;

	/* configure mock */
	will_return(rdma_create_event_channel, MOCK_EVCH);
	Rdma_migrate_id_counter = RDMA_MIGRATE_COUNTER_INIT;
	will_return(rdma_migrate_id, MOCK_OK);
	will_return(rpma_flush_new, MOCK_OK);
	will_return(__wrap__test_malloc, MOCK_OK);

	/* prepare an object */
	int ret = rpma_conn_new(MOCK_PEER, MOCK_CM_ID,
			MOCK_RPMA_CQ, cstate->rcq, &cstate->conn);

	/* verify the results */
	assert_int_equal(ret, MOCK_OK);
	assert_non_null(cstate->conn);

	*cstate_ptr = cstate;

	return 0;
}

/*
 * teardown__conn_delete - delete the rpma_conn object
 */
int
teardown__conn_delete(void **cstate_ptr)
{
	struct conn_test_state *cstate = *cstate_ptr;

	/* configure mocks: */
	will_return(rpma_flush_delete, MOCK_OK);
	expect_value(rdma_destroy_qp, id, MOCK_CM_ID);
	expect_value(rpma_cq_delete, *cq_ptr, cstate->rcq);
	will_return(rpma_cq_delete, MOCK_OK);
	expect_value(rpma_cq_delete, *cq_ptr, MOCK_RPMA_CQ);
	will_return(rpma_cq_delete, MOCK_OK);
	expect_value(rdma_destroy_id, id, MOCK_CM_ID);
	will_return(rdma_destroy_id, MOCK_OK);
	expect_value(rpma_private_data_discard, pdata->ptr,
				cstate->data.ptr);
	expect_value(rpma_private_data_discard, pdata->len,
				cstate->data.len);

	/* delete the object */
	int ret = rpma_conn_delete(&cstate->conn);

	/* verify the results */
	assert_int_equal(ret, MOCK_OK);
	assert_null(cstate->conn);

	*cstate_ptr = NULL;

	return 0;
}
