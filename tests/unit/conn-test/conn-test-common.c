/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2020, Intel Corporation
 */

/*
 * conn-test-common.c -- the connection unit tests common functions
 */

#include "conn-test-common.h"
#include "mocks-rdma_cm.h"
#include "test-common.h"

/*
 * conn_setup - prepare a valid rpma_conn object
 */
int
conn_setup(void **cstate_ptr)
{
	static struct conn_test_state cstate;
	cstate.conn = NULL;
	cstate.data.ptr = NULL;
	cstate.data.len = 0;

	/* configure mock: */
	will_return(rdma_create_event_channel, MOCK_EVCH);
	Rdma_migrate_id_counter = RDMA_MIGRATE_COUNTER_INIT;
	will_return(rdma_migrate_id, MOCK_OK);
	will_return(__wrap__test_malloc, MOCK_OK);

	/* prepare an object */
	int ret = rpma_conn_new(MOCK_PEER, MOCK_CM_ID,
			MOCK_IBV_CQ, &cstate.conn);

	/* verify the results */
	assert_int_equal(ret, MOCK_OK);
	assert_non_null(cstate.conn);

	*cstate_ptr = &cstate;

	return 0;
}

/*
 * conn_teardown - delete the rpma_conn object
 */
int
conn_teardown(void **cstate_ptr)
{
	struct conn_test_state *cstate = *cstate_ptr;

	/* configure mocks: */
	expect_value(rdma_destroy_qp, id, MOCK_CM_ID);
	will_return(ibv_destroy_cq, MOCK_OK);
	expect_value(rdma_destroy_id, id, MOCK_CM_ID);
	will_return(rdma_destroy_id, MOCK_OK);
	expect_value(rpma_private_data_discard, pdata->ptr, cstate->data.ptr);
	expect_value(rpma_private_data_discard, pdata->len, cstate->data.len);

	/* delete the object */
	int ret = rpma_conn_delete(&cstate->conn);

	/* verify the results */
	assert_int_equal(ret, MOCK_OK);
	assert_null(cstate->conn);

	*cstate_ptr = NULL;

	return 0;
}
