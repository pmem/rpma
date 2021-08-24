// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020, Intel Corporation */
/* Copyright 2021, Fujitsu */

/*
 * conn_req-from_cm_event.c -- the rpma_conn_req_from_cm_event() unit tests
 *
 * API covered:
 * - rpma_conn_req_from_cm_event()
 */

#include "conn_req-common.h"
#include "mocks-ibverbs.h"
#include "mocks-rdma_cm.h"
#include "mocks-rpma-conn_cfg.h"

/*
 * from_cm_event__peer_NULL -- NULL peer is invalid
 */
static void
from_cm_event__peer_NULL(void **unused)
{
	/* run test */
	struct rdma_cm_event event = CM_EVENT_CONNECTION_REQUEST_INIT;
	struct rpma_conn_req *req = NULL;
	int ret = rpma_conn_req_from_cm_event(NULL, &event,
			MOCK_CONN_CFG_DEFAULT, &req);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
	assert_null(req);
}

/*
 * from_cm_event__edata_NULL -- NULL edata is invalid
 */
static void
from_cm_event__edata_NULL(void **unused)
{
	/* run test */
	struct rpma_conn_req *req = NULL;
	int ret = rpma_conn_req_from_cm_event(MOCK_PEER, NULL,
			MOCK_CONN_CFG_DEFAULT, &req);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
	assert_null(req);
}

/*
 * from_cm_event__req_ptr_NULL -- NULL req_ptr is invalid
 */
static void
from_cm_event__req_ptr_NULL(void **unused)
{
	/* run test */
	struct rdma_cm_event event = CM_EVENT_CONNECTION_REQUEST_INIT;
	int ret = rpma_conn_req_from_cm_event(MOCK_PEER, &event,
			MOCK_CONN_CFG_DEFAULT, NULL);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * from_cm_event__peer_NULL_edata_NULL_req_ptr_NULL -- NULL peer,
 * NULL edata and NULL req_ptr are not valid
 */
static void
from_cm_event__peer_NULL_edata_NULL_req_ptr_NULL(void **unused)
{
	/* run test */
	int ret = rpma_conn_req_from_cm_event(NULL, NULL,
			MOCK_CONN_CFG_DEFAULT, NULL);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * from_cm_event__RDMA_CM_EVENT_CONNECT_ERROR -- edata of type
 * RDMA_CM_EVENT_CONNECT_ERROR
 */
static void
from_cm_event__RDMA_CM_EVENT_CONNECT_ERROR(void **unused)
{
	/* run test */
	struct rdma_cm_event event = CM_EVENT_CONNECT_ERROR_INIT;
	struct rpma_conn_req *req = NULL;
	int ret = rpma_conn_req_from_cm_event(MOCK_PEER, &event,
			MOCK_CONN_CFG_DEFAULT, &req);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
	assert_null(req);
}

/*
 * from_cm_event__cq_new_ERRNO -- rpma_cq_new(cqe) fails with MOCK_ERRNO
 */
static void
from_cm_event__cq_new_ERRNO(void **cstate_ptr)
{
	struct conn_req_test_state *cstate = *cstate_ptr;
	configure_conn_req((void **)&cstate);

	/* configure mocks */
	will_return(rpma_conn_cfg_get_cqe, &cstate->get_cqe);
	will_return(rpma_conn_cfg_get_rcqe, &cstate->get_cqe);
	expect_value(rpma_cq_new, cqe, cstate->get_cqe.cq_size);
	will_return(rpma_cq_new, NULL);
	will_return(rpma_cq_new, RPMA_E_PROVIDER);
	will_return(rpma_cq_new, MOCK_ERRNO);

	/* run test */
	struct rpma_conn_req *req = NULL;
	int ret = rpma_conn_req_from_cm_event(MOCK_PEER, &cstate->event,
			cstate->get_cqe.cfg, &req);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_null(req);
}

/*
 * from_cm_event__rcq_new_ERRNO -- rpma_cq_new(rcqe) fails with MOCK_ERRNO
 */
static void
from_cm_event__rcq_new_ERRNO(void **unused)
{
	/* rpma_cq_new(rcqe) is called only when rcqe > 0 */
	struct conn_req_test_state *cstate = &Conn_req_conn_cfg_custom;
	configure_conn_req((void **)&cstate);

	/* configure mocks */
	will_return(rpma_conn_cfg_get_cqe, &cstate->get_cqe);
	will_return(rpma_conn_cfg_get_rcqe, &cstate->get_cqe);
	expect_value(rpma_cq_new, cqe, cstate->get_cqe.cq_size);
	will_return(rpma_cq_new, MOCK_RPMA_CQ);
	expect_value(rpma_cq_new, cqe, cstate->get_cqe.rcq_size);
	will_return(rpma_cq_new, NULL);
	will_return(rpma_cq_new, RPMA_E_PROVIDER);
	will_return(rpma_cq_new, MOCK_ERRNO);
	expect_value(rpma_cq_delete, *cq_ptr, MOCK_RPMA_CQ);
	will_return(rpma_cq_delete, MOCK_OK);

	/* run test */
	struct rpma_conn_req *req = NULL;
	int ret = rpma_conn_req_from_cm_event(MOCK_PEER, &cstate->event,
			cstate->get_cqe.cfg, &req);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_null(req);
}

/*
 * from_cm_event__peer_create_qp_ERRNO -- rpma_peer_create_qp()
 * fails with MOCK_ERRNO
 */
static void
from_cm_event__peer_create_qp_ERRNO(void **cstate_ptr)
{
	struct conn_req_test_state *cstate = *cstate_ptr;
	configure_conn_req((void **)&cstate);

	/* configure mocks */
	will_return(rpma_conn_cfg_get_cqe, &cstate->get_cqe);
	will_return(rpma_conn_cfg_get_rcqe, &cstate->get_cqe);
	expect_value(rpma_cq_new, cqe, cstate->get_cqe.cq_size);
	will_return(rpma_cq_new, MOCK_RPMA_CQ);
	if (cstate->get_cqe.rcq_size) {
		expect_value(rpma_cq_new, cqe, cstate->get_cqe.rcq_size);
		will_return(rpma_cq_new, MOCK_RPMA_RCQ);
	}
	expect_value(rpma_peer_create_qp, id, &cstate->id);
	expect_value(rpma_peer_create_qp, rcq, MOCK_GET_RCQ(cstate));
	expect_value(rpma_peer_create_qp, cfg, cstate->get_cqe.cfg);
	will_return(rpma_peer_create_qp, RPMA_E_PROVIDER);
	will_return(rpma_peer_create_qp, MOCK_ERRNO);
	expect_value(rpma_cq_delete, *cq_ptr, MOCK_GET_RCQ(cstate));
	will_return(rpma_cq_delete, MOCK_OK);
	expect_value(rpma_cq_delete, *cq_ptr, MOCK_RPMA_CQ);
	will_return(rpma_cq_delete, MOCK_OK);

	/* run test */
	struct rpma_conn_req *req = NULL;
	int ret = rpma_conn_req_from_cm_event(MOCK_PEER, &cstate->event,
			cstate->get_cqe.cfg, &req);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_null(req);
}

/*
 * from_cm_event__create_qp_ERRNO_subsequent_ERRNO2 -- rpma_peer_create_qp()
 * fails with MOCK_ERRNO whereas subsequent (rpma_cq_delete(&rcq),
 * rpma_cq_delete(&cq)) fail with MOCK_ERRNO2
 */
static void
from_cm_event__create_qp_ERRNO_subsequent_ERRNO2(void **cstate_ptr)
{
	struct conn_req_test_state *cstate = *cstate_ptr;
	configure_conn_req((void **)&cstate);

	/* configure mocks */
	will_return(rpma_conn_cfg_get_cqe, &cstate->get_cqe);
	will_return(rpma_conn_cfg_get_rcqe, &cstate->get_cqe);
	expect_value(rpma_cq_new, cqe, cstate->get_cqe.cq_size);
	will_return(rpma_cq_new, MOCK_RPMA_CQ);
	if (cstate->get_cqe.rcq_size) {
		expect_value(rpma_cq_new, cqe, cstate->get_cqe.rcq_size);
		will_return(rpma_cq_new, MOCK_RPMA_RCQ);
	}
	expect_value(rpma_peer_create_qp, id, &cstate->id);
	expect_value(rpma_peer_create_qp, rcq, MOCK_GET_RCQ(cstate));
	expect_value(rpma_peer_create_qp, cfg, cstate->get_cqe.cfg);
	will_return(rpma_peer_create_qp, RPMA_E_PROVIDER);
	will_return(rpma_peer_create_qp, MOCK_ERRNO); /* first error */
	expect_value(rpma_cq_delete, *cq_ptr, MOCK_GET_RCQ(cstate));
	if (cstate->get_cqe.rcq_size) {
		will_return(rpma_cq_delete, RPMA_E_PROVIDER);
		will_return(rpma_cq_delete, MOCK_ERRNO2); /* second error */
	} else {
		/* rcq == NULL cannot fail */
		will_return(rpma_cq_delete, MOCK_OK);
	}
	expect_value(rpma_cq_delete, *cq_ptr, MOCK_RPMA_CQ);
	will_return(rpma_cq_delete, RPMA_E_PROVIDER);
	will_return(rpma_cq_delete, MOCK_ERRNO2); /* second or third error */

	/* run test */
	struct rpma_conn_req *req = NULL;
	int ret = rpma_conn_req_from_cm_event(MOCK_PEER, &cstate->event,
			cstate->get_cqe.cfg, &req);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_null(req);
}

/*
 * from_cm_event__malloc_ERRNO -- malloc() fails with MOCK_ERRNO
 */
static void
from_cm_event__malloc_ERRNO(void **cstate_ptr)
{
	struct conn_req_test_state *cstate = *cstate_ptr;
	configure_conn_req((void **)&cstate);

	/* configure mocks */
	will_return(rpma_conn_cfg_get_cqe, &cstate->get_cqe);
	will_return(rpma_conn_cfg_get_rcqe, &cstate->get_cqe);
	expect_value(rpma_cq_new, cqe, cstate->get_cqe.cq_size);
	will_return(rpma_cq_new, MOCK_RPMA_CQ);
	if (cstate->get_cqe.rcq_size) {
		expect_value(rpma_cq_new, cqe, cstate->get_cqe.rcq_size);
		will_return(rpma_cq_new, MOCK_RPMA_RCQ);
	}
	expect_value(rpma_peer_create_qp, id, &cstate->id);
	expect_value(rpma_peer_create_qp, rcq, MOCK_GET_RCQ(cstate));
	expect_value(rpma_peer_create_qp, cfg, cstate->get_cqe.cfg);
	will_return(rpma_peer_create_qp, MOCK_OK);
	will_return(__wrap__test_malloc, MOCK_ERRNO);
	expect_value(rdma_destroy_qp, id, &cstate->id);
	expect_value(rpma_cq_delete, *cq_ptr, MOCK_GET_RCQ(cstate));
	will_return(rpma_cq_delete, MOCK_OK);
	expect_value(rpma_cq_delete, *cq_ptr, MOCK_RPMA_CQ);
	will_return(rpma_cq_delete, MOCK_OK);

	/* run test */
	struct rpma_conn_req *req = NULL;
	int ret = rpma_conn_req_from_cm_event(MOCK_PEER, &cstate->event,
			cstate->get_cqe.cfg, &req);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_NOMEM);
	assert_null(req);
}

/*
 * from_cm_event__malloc_ERRNO_subsequent_ERRNO2 -- malloc() fails
 * with MOCK_ERRNO whereas subsequent (rpma_cq_delete(&rcq),
 * rpma_cq_delete(&cq)) fail with MOCK_ERRNO2
 */
static void
from_cm_event__malloc_ERRNO_subsequent_ERRNO2(void **cstate_ptr)
{
	struct conn_req_test_state *cstate = *cstate_ptr;
	configure_conn_req((void **)&cstate);

	/* configure mocks */
	will_return(rpma_conn_cfg_get_cqe, &cstate->get_cqe);
	will_return(rpma_conn_cfg_get_rcqe, &cstate->get_cqe);
	expect_value(rpma_cq_new, cqe, cstate->get_cqe.cq_size);
	will_return(rpma_cq_new, MOCK_RPMA_CQ);
	if (cstate->get_cqe.rcq_size) {
		expect_value(rpma_cq_new, cqe, cstate->get_cqe.rcq_size);
		will_return(rpma_cq_new, MOCK_RPMA_RCQ);
	}
	expect_value(rpma_peer_create_qp, id, &cstate->id);
	expect_value(rpma_peer_create_qp, rcq, MOCK_GET_RCQ(cstate));
	expect_value(rpma_peer_create_qp, cfg, cstate->get_cqe.cfg);
	will_return(rpma_peer_create_qp, MOCK_OK);
	will_return(__wrap__test_malloc, MOCK_ERRNO); /* first error */
	expect_value(rdma_destroy_qp, id, &cstate->id);
	expect_value(rpma_cq_delete, *cq_ptr, MOCK_GET_RCQ(cstate));
	if (cstate->get_cqe.rcq_size) {
		will_return(rpma_cq_delete, RPMA_E_PROVIDER);
		will_return(rpma_cq_delete, MOCK_ERRNO2); /* second error */
	} else {
		/* rcq == NULL cannot fail */
		will_return(rpma_cq_delete, MOCK_OK);
	}
	expect_value(rpma_cq_delete, *cq_ptr, MOCK_RPMA_CQ);
	will_return(rpma_cq_delete, RPMA_E_PROVIDER);
	will_return(rpma_cq_delete, MOCK_ERRNO2); /* second or third error */

	/* run test */
	struct rpma_conn_req *req = NULL;
	int ret = rpma_conn_req_from_cm_event(MOCK_PEER, &cstate->event,
			cstate->get_cqe.cfg, &req);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_NOMEM);
	assert_null(req);
}

/*
 * from_cm_event__private_data_store_E_NOMEM -- rpma_private_data_store()
 * fails with RPMA_E_NOMEM
 */
static void
from_cm_event__private_data_store_E_NOMEM(void **cstate_ptr)
{
	struct conn_req_test_state *cstate = *cstate_ptr;
	configure_conn_req((void **)&cstate);

	/* configure mocks */
	will_return(rpma_conn_cfg_get_cqe, &cstate->get_cqe);
	will_return(rpma_conn_cfg_get_rcqe, &cstate->get_cqe);
	expect_value(rpma_cq_new, cqe, cstate->get_cqe.cq_size);
	will_return(rpma_cq_new, MOCK_RPMA_CQ);
	if (cstate->get_cqe.rcq_size) {
		expect_value(rpma_cq_new, cqe, cstate->get_cqe.rcq_size);
		will_return(rpma_cq_new, MOCK_RPMA_RCQ);
	}
	expect_value(rpma_peer_create_qp, id, &cstate->id);
	expect_value(rpma_peer_create_qp, rcq, MOCK_GET_RCQ(cstate));
	expect_value(rpma_peer_create_qp, cfg, cstate->get_cqe.cfg);
	will_return(rpma_peer_create_qp, MOCK_OK);
	will_return(__wrap__test_malloc, MOCK_OK);
	will_return(rpma_private_data_store, NULL); /* RPMA_E_NOMEM */
	expect_value(rdma_destroy_qp, id, &cstate->id);
	expect_value(rpma_cq_delete, *cq_ptr, MOCK_GET_RCQ(cstate));
	will_return(rpma_cq_delete, MOCK_OK);
	expect_value(rpma_cq_delete, *cq_ptr, MOCK_RPMA_CQ);
	will_return(rpma_cq_delete, MOCK_OK);
	expect_value(rdma_destroy_id, id, &cstate->id);
	will_return(rdma_destroy_id, MOCK_OK);
	expect_function_call(rpma_private_data_discard);

	/* run test */
	struct rpma_conn_req *req = NULL;
	int ret = rpma_conn_req_from_cm_event(MOCK_PEER, &cstate->event,
			cstate->get_cqe.cfg, &req);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_NOMEM);
	assert_null(req);
}

/*
 * from_cm_event__private_data_store_E_NOMEM_subsequent_ERRNO2 --
 * rpma_private_data_store() fails with RPMA_E_NOMEM whereas subsequent
 * (rpma_cq_delete(&rcq), rpma_cq_delete(&cq), rdma_destroy_id()) fail
 * with MOCK_ERRNO2
 */
static void
from_cm_event__private_data_store_E_NOMEM_subsequent_ERRNO2(void **cstate_ptr)
{
	struct conn_req_test_state *cstate = *cstate_ptr;
	configure_conn_req((void **)&cstate);

	/* configure mocks */
	will_return(rpma_conn_cfg_get_cqe, &cstate->get_cqe);
	will_return(rpma_conn_cfg_get_rcqe, &cstate->get_cqe);
	expect_value(rpma_cq_new, cqe, cstate->get_cqe.cq_size);
	will_return(rpma_cq_new, MOCK_RPMA_CQ);
	if (cstate->get_cqe.rcq_size) {
		expect_value(rpma_cq_new, cqe, cstate->get_cqe.rcq_size);
		will_return(rpma_cq_new, MOCK_RPMA_RCQ);
	}
	expect_value(rpma_peer_create_qp, id, &cstate->id);
	expect_value(rpma_peer_create_qp, rcq, MOCK_GET_RCQ(cstate));
	expect_value(rpma_peer_create_qp, cfg, cstate->get_cqe.cfg);
	will_return(rpma_peer_create_qp, MOCK_OK);
	will_return(__wrap__test_malloc, MOCK_OK);
	will_return(rpma_private_data_store, NULL); /* first RPMA_E_NOMEM */
	expect_value(rdma_destroy_qp, id, &cstate->id);
	expect_value(rpma_cq_delete, *cq_ptr, MOCK_GET_RCQ(cstate));
	if (cstate->get_cqe.rcq_size) {
		will_return(rpma_cq_delete, RPMA_E_PROVIDER);
		will_return(rpma_cq_delete, MOCK_ERRNO2); /* second error */
	} else {
		/* rcq == NULL cannot fail */
		will_return(rpma_cq_delete, MOCK_OK);
	}
	expect_value(rpma_cq_delete, *cq_ptr, MOCK_RPMA_CQ);
	will_return(rpma_cq_delete, RPMA_E_PROVIDER);
	will_return(rpma_cq_delete, MOCK_ERRNO2); /* second or third error */
	expect_value(rdma_destroy_id, id, &cstate->id);
	will_return(rdma_destroy_id, MOCK_ERRNO2); /* third or fourth error */
	expect_function_call(rpma_private_data_discard);

	/* run test */
	struct rpma_conn_req *req = NULL;
	int ret = rpma_conn_req_from_cm_event(MOCK_PEER, &cstate->event,
			cstate->get_cqe.cfg, &req);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_NOMEM);
	assert_null(req);
}

/*
 * conn_req_from_cm__lifecycle - happy day scenario
 */
static void
conn_req_from_cm__lifecycle(void **unused)
{
	/*
	 * The thing is done by setup__conn_req_from_cm_event() and
	 * teardown__conn_req_from_cm_event().
	 */
}

static const struct CMUnitTest test_from_cm_event[] = {
	/* rpma_conn_req_from_cm_event() unit tests */
	cmocka_unit_test(from_cm_event__peer_NULL),
	cmocka_unit_test(from_cm_event__edata_NULL),
	cmocka_unit_test(from_cm_event__req_ptr_NULL),
	cmocka_unit_test(
		from_cm_event__peer_NULL_edata_NULL_req_ptr_NULL),
	cmocka_unit_test(
		from_cm_event__RDMA_CM_EVENT_CONNECT_ERROR),
	CONN_REQ_TEST_WITH_AND_WITHOUT_RCQ(from_cm_event__cq_new_ERRNO),
	cmocka_unit_test(from_cm_event__rcq_new_ERRNO),
	CONN_REQ_TEST_WITH_AND_WITHOUT_RCQ(
		from_cm_event__peer_create_qp_ERRNO),
	CONN_REQ_TEST_WITH_AND_WITHOUT_RCQ(
		from_cm_event__create_qp_ERRNO_subsequent_ERRNO2),
	CONN_REQ_TEST_WITH_AND_WITHOUT_RCQ(from_cm_event__malloc_ERRNO),
	CONN_REQ_TEST_WITH_AND_WITHOUT_RCQ(
		from_cm_event__malloc_ERRNO_subsequent_ERRNO2),
	CONN_REQ_TEST_WITH_AND_WITHOUT_RCQ(
		from_cm_event__private_data_store_E_NOMEM),
	CONN_REQ_TEST_WITH_AND_WITHOUT_RCQ(
		from_cm_event__private_data_store_E_NOMEM_subsequent_ERRNO2),
	/* rpma_conn_req_from_cm_event()/_delete() lifecycle */
	CONN_REQ_TEST_SETUP_TEARDOWN_WITH_AND_WITHOUT_RCQ(
		conn_req_from_cm__lifecycle, setup__conn_req_from_cm_event,
		teardown__conn_req_from_cm_event),
};

int
main(int argc, char *argv[])
{
	return cmocka_run_group_tests(test_from_cm_event, NULL, NULL);
}
