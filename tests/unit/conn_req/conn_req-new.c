// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020, Intel Corporation */
/* Copyright 2021, Fujitsu */

/*
 * conn_req-new.c -- the rpma_conn_req_new() unit tests
 *
 * API covered:
 * - rpma_conn_req_new()
 */

#include "conn_req-common.h"
#include "mocks-ibverbs.h"
#include "mocks-rpma-conn_cfg.h"

static struct conn_cfg_get_timeout_mock_args Get_t = {
	.cfg = MOCK_CONN_CFG_DEFAULT,
	.timeout_ms = RPMA_DEFAULT_TIMEOUT_MS
};

static struct conn_cfg_get_q_size_mock_args Get_cqe = {
		.cfg = MOCK_CONN_CFG_DEFAULT,
		.q_size = MOCK_CQ_SIZE_DEFAULT
};

/*
 * new__peer_NULL -- NULL peer is invalid
 */
static void
new__peer_NULL(void **unused)
{
	/* run test */
	struct rpma_conn_req *req = NULL;
	int ret = rpma_conn_req_new(NULL, MOCK_IP_ADDRESS, MOCK_PORT, NULL,
			&req);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
	assert_null(req);
}

/*
 * new__addr_NULL -- NULL addr is invalid
 */
static void
new__addr_NULL(void **unused)
{
	/* run test */
	struct rpma_conn_req *req = NULL;
	int ret = rpma_conn_req_new(MOCK_PEER, NULL, MOCK_PORT, NULL, &req);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
	assert_null(req);
}

/*
 * new__port_NULL -- NULL port is invalid
 */
static void
new__port_NULL(void **unused)
{
	/* run test */
	struct rpma_conn_req *req = NULL;
	int ret = rpma_conn_req_new(MOCK_PEER, MOCK_IP_ADDRESS, NULL, NULL,
			&req);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
	assert_null(req);
}

/*
 * new__req_ptr_NULL -- NULL req_ptr is invalid
 */
static void
new__req_ptr_NULL(void **unused)
{
	/* run test */
	int ret = rpma_conn_req_new(MOCK_PEER, MOCK_IP_ADDRESS, MOCK_PORT, NULL,
					NULL);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * new__all_NULL -- all NULL arguments are invalid
 */
static void
new__all_NULL(void **unused)
{
	/* run test */
	int ret = rpma_conn_req_new(NULL, NULL, NULL, NULL, NULL);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * new__info_new_ERRNO -- rpma_info_new() fails with MOCK_ERRNO
 */
static void
new__info_new_ERRNO(void **unused)
{
	struct rdma_cm_id id = {0};

	/* configure mocks */
	will_return(rpma_conn_cfg_get_timeout, &Get_t);
	will_return(rpma_info_new, NULL);
	will_return(rpma_info_new, RPMA_E_PROVIDER);
	will_return(rpma_info_new, MOCK_ERRNO);
	will_return_maybe(rdma_create_id, &id);
	will_return_maybe(rdma_destroy_id, MOCK_OK);

	/* run test */
	struct rpma_conn_req *req = NULL;
	int ret = rpma_conn_req_new(MOCK_PEER, MOCK_IP_ADDRESS, MOCK_PORT, NULL,
			&req);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_null(req);
}

/*
 * new__create_id_ERRNO -- rdma_create_id() fails with MOCK_ERRNO
 */
static void
new__create_id_ERRNO(void **unused)
{
	/* configure mocks */
	will_return(rpma_conn_cfg_get_timeout, &Get_t);
	will_return(rdma_create_id, NULL);
	will_return(rdma_create_id, MOCK_ERRNO);
	will_return_maybe(rpma_info_new, MOCK_INFO);

	/* run test */
	struct rpma_conn_req *req = NULL;
	int ret = rpma_conn_req_new(MOCK_PEER, MOCK_IP_ADDRESS, MOCK_PORT, NULL,
			&req);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_null(req);
}

/*
 * new__resolve_addr_ERRNO -- rpma_info_resolve_addr() fails
 * with MOCK_ERRNO
 */
static void
new__resolve_addr_ERRNO(void **unused)
{
	struct rdma_cm_id id = {0};
	id.verbs = MOCK_VERBS;

	/* configure mocks */
	will_return(rpma_conn_cfg_get_timeout, &Get_t);
	will_return(rpma_info_new, MOCK_INFO);
	will_return(rdma_create_id, &id);
	expect_value(rpma_info_resolve_addr, id, &id);
	expect_value(rpma_info_resolve_addr, timeout_ms,
			RPMA_DEFAULT_TIMEOUT_MS);
	will_return(rpma_info_resolve_addr, RPMA_E_PROVIDER);
	will_return(rpma_info_resolve_addr, MOCK_ERRNO);
	will_return(rdma_destroy_id, MOCK_OK);

	/* run test */
	struct rpma_conn_req *req = NULL;
	int ret = rpma_conn_req_new(MOCK_PEER, MOCK_IP_ADDRESS, MOCK_PORT, NULL,
			&req);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_null(req);
}

/*
 * new__resolve_route_ERRNO -- rdma_resolve_route() fails with MOCK_ERRNO
 */
static void
new__resolve_route_ERRNO(void **unused)
{
	struct rdma_cm_id id = {0};
	id.verbs = MOCK_VERBS;

	/* configure mocks */
	will_return(rpma_conn_cfg_get_timeout, &Get_t);
	will_return(rpma_info_new, MOCK_INFO);
	will_return(rdma_create_id, &id);
	expect_value(rpma_info_resolve_addr, id, &id);
	expect_value(rpma_info_resolve_addr, timeout_ms,
			RPMA_DEFAULT_TIMEOUT_MS);
	will_return(rpma_info_resolve_addr, MOCK_OK);
	/*
	 * XXX rdma_resolve_route() mock assumes all its expects comes from
	 * another mock. The following expect breaks this assumption.
	 */
	expect_value(rdma_resolve_route, timeout_ms, RPMA_DEFAULT_TIMEOUT_MS);
	will_return(rdma_resolve_route, MOCK_ERRNO);
	will_return(rdma_destroy_id, MOCK_OK);

	/* run test */
	struct rpma_conn_req *req = NULL;
	int ret = rpma_conn_req_new(MOCK_PEER, MOCK_IP_ADDRESS, MOCK_PORT, NULL,
			&req);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_null(req);
}

/*
 * new__cq_new_ERRNO -- rpma_cq_new() fails with MOCK_ERRNO
 */
static void
new__cq_new_ERRNO(void **unused)
{
	struct rdma_cm_id id = {0};
	id.verbs = MOCK_VERBS;

	/* configure mocks */
	will_return(rpma_conn_cfg_get_timeout, &Get_t);
	will_return(rpma_info_new, MOCK_INFO);
	will_return(rdma_create_id, &id);
	expect_value(rpma_info_resolve_addr, id, &id);
	expect_value(rpma_info_resolve_addr, timeout_ms,
				RPMA_DEFAULT_TIMEOUT_MS);
	will_return(rpma_info_resolve_addr, MOCK_OK);
	/*
	 * XXX rdma_resolve_route() mock assumes all its expects comes from
	 * another mock. The following expect breaks this assumption.
	 */
	expect_value(rdma_resolve_route, timeout_ms, RPMA_DEFAULT_TIMEOUT_MS);
	will_return(rdma_resolve_route, MOCK_OK);
	will_return(rpma_conn_cfg_get_cqe, &Get_cqe);
	expect_value(rpma_cq_new, cqe, MOCK_CQ_SIZE_DEFAULT);
	will_return(rpma_cq_new, NULL);
	will_return(rpma_cq_new, RPMA_E_PROVIDER);
	will_return(rpma_cq_new, MOCK_ERRNO);
	will_return(rdma_destroy_id, MOCK_OK);
	will_return_maybe(rpma_conn_cfg_get_cqe, &Get_cqe);

	/* run test */
	struct rpma_conn_req *req = NULL;
	int ret = rpma_conn_req_new(MOCK_PEER, MOCK_IP_ADDRESS, MOCK_PORT, NULL,
			&req);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_null(req);
}

/*
 * new__peer_create_qp_ERRNO -- rpma_peer_create_qp() fails
 * with MOCK_ERRNO
 */
static void
new__peer_create_qp_ERRNO(void **unused)
{
	struct rdma_cm_id id = {0};
	id.verbs = MOCK_VERBS;

	/* configure mocks */
	will_return(rpma_conn_cfg_get_timeout, &Get_t);
	will_return(rpma_info_new, MOCK_INFO);
	will_return(rdma_create_id, &id);
	expect_value(rpma_info_resolve_addr, id, &id);
	expect_value(rpma_info_resolve_addr, timeout_ms,
				RPMA_DEFAULT_TIMEOUT_MS);
	will_return(rpma_info_resolve_addr, MOCK_OK);
	/*
	 * XXX rdma_resolve_route() mock assumes all its expects comes from
	 * another mock. The following expect breaks this assumption.
	 */
	expect_value(rdma_resolve_route, timeout_ms, RPMA_DEFAULT_TIMEOUT_MS);
	will_return(rdma_resolve_route, MOCK_OK);
	will_return(rpma_conn_cfg_get_cqe, &Get_cqe);
	expect_value(rpma_cq_new, cqe, MOCK_CQ_SIZE_DEFAULT);
	will_return(rpma_cq_new, MOCK_RPMA_CQ);
	expect_value(rpma_peer_create_qp, id, &id);
	expect_value(rpma_peer_create_qp, cfg, MOCK_CONN_CFG_DEFAULT);
	will_return(rpma_peer_create_qp, RPMA_E_PROVIDER);
	will_return(rpma_peer_create_qp, MOCK_ERRNO);
	will_return(rpma_cq_delete, MOCK_OK);
	will_return(rdma_destroy_id, MOCK_OK);

	/* run test */
	struct rpma_conn_req *req = NULL;
	int ret = rpma_conn_req_new(MOCK_PEER, MOCK_IP_ADDRESS, MOCK_PORT, NULL,
			&req);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_null(req);
}

/*
 * new__malloc_ERRNO -- malloc() fails with MOCK_ERRNO
 */
static void
new__malloc_ERRNO(void **unused)
{
	struct rdma_cm_id id = {0};
	id.verbs = MOCK_VERBS;

	/* configure mocks */
	will_return(rpma_conn_cfg_get_timeout, &Get_t);
	will_return(rpma_info_new, MOCK_INFO);
	will_return(rdma_create_id, &id);
	expect_value(rpma_info_resolve_addr, id, &id);
	expect_value(rpma_info_resolve_addr, timeout_ms,
				RPMA_DEFAULT_TIMEOUT_MS);
	will_return(rpma_info_resolve_addr, MOCK_OK);
	/*
	 * XXX rdma_resolve_route() mock assumes all its expects comes from
	 * another mock. The following expect breaks this assumption.
	 */
	expect_value(rdma_resolve_route, timeout_ms, RPMA_DEFAULT_TIMEOUT_MS);
	will_return(rdma_resolve_route, MOCK_OK);
	will_return(rpma_conn_cfg_get_cqe, &Get_cqe);
	expect_value(rpma_cq_new, cqe, MOCK_CQ_SIZE_DEFAULT);
	will_return(rpma_cq_new, MOCK_RPMA_CQ);
	expect_value(rpma_peer_create_qp, id, &id);
	expect_value(rpma_peer_create_qp, cfg, MOCK_CONN_CFG_DEFAULT);
	will_return(rpma_peer_create_qp, MOCK_OK);
	will_return(__wrap__test_malloc, MOCK_ERRNO);
	expect_value(rdma_destroy_qp, id, &id);
	will_return(rpma_cq_delete, MOCK_OK);
	will_return(rdma_destroy_id, MOCK_OK);

	/* run test */
	struct rpma_conn_req *req = NULL;
	int ret = rpma_conn_req_new(MOCK_PEER, MOCK_IP_ADDRESS, MOCK_PORT, NULL,
			&req);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_NOMEM);
	assert_null(req);
}

/*
 * new__malloc_ERRNO_subsequent_ERRNO2 -- malloc() fails with MOCK_ERRNO
 * whereas subsequent calls fail with MOCK_ERRNO2
 */
static void
new__malloc_ERRNO_subsequent_ERRNO2(void **unused)
{
	struct rdma_cm_id id = {0};
	id.verbs = MOCK_VERBS;

	/* configure mocks */
	will_return(rpma_conn_cfg_get_timeout, &Get_t);
	will_return(rpma_info_new, MOCK_INFO);
	will_return(rdma_create_id, &id);
	expect_value(rpma_info_resolve_addr, id, &id);
	expect_value(rpma_info_resolve_addr, timeout_ms,
				RPMA_DEFAULT_TIMEOUT_MS);
	will_return(rpma_info_resolve_addr, MOCK_OK);
	/*
	 * XXX rdma_resolve_route() mock assumes all its expects comes from
	 * another mock. The following expect breaks this assumption.
	 */
	expect_value(rdma_resolve_route, timeout_ms, RPMA_DEFAULT_TIMEOUT_MS);
	will_return(rdma_resolve_route, MOCK_OK);
	will_return(rpma_conn_cfg_get_cqe, &Get_cqe);
	expect_value(rpma_cq_new, cqe, MOCK_CQ_SIZE_DEFAULT);
	will_return(rpma_cq_new, MOCK_RPMA_CQ);
	expect_value(rpma_peer_create_qp, id, &id);
	expect_value(rpma_peer_create_qp, cfg, MOCK_CONN_CFG_DEFAULT);
	will_return(rpma_peer_create_qp, MOCK_OK);
	will_return(__wrap__test_malloc, MOCK_ERRNO); /* first error */
	expect_value(rdma_destroy_qp, id, &id);
	will_return(rpma_cq_delete, MOCK_ERRNO2); /* second error */
	will_return(rdma_destroy_id, MOCK_ERRNO2); /* third error */

	/* run test */
	struct rpma_conn_req *req = NULL;
	int ret = rpma_conn_req_new(MOCK_PEER, MOCK_IP_ADDRESS, MOCK_PORT, NULL,
			&req);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_NOMEM);
	assert_null(req);
}

/*
 * new__success -- all is OK
 */
static void
new__success(void **unused)
{
	/*
	 * The thing is done by setup__conn_req_new() and
	 * teardown__conn_req_new().
	 */
}

static struct conn_req_new_test_state prestate_conn_cfg_default;
static struct conn_req_new_test_state prestate_conn_cfg_custom;

static const struct CMUnitTest test_new[] = {
	/* rpma_conn_req_new() unit tests */
	cmocka_unit_test(new__peer_NULL),
	cmocka_unit_test(new__addr_NULL),
	cmocka_unit_test(new__port_NULL),
	cmocka_unit_test(new__req_ptr_NULL),
	cmocka_unit_test(new__all_NULL),
	cmocka_unit_test(new__info_new_ERRNO),
	cmocka_unit_test(new__create_id_ERRNO),
	cmocka_unit_test(new__resolve_addr_ERRNO),
	cmocka_unit_test(new__resolve_route_ERRNO),
	cmocka_unit_test(new__cq_new_ERRNO),
	cmocka_unit_test(new__peer_create_qp_ERRNO),
	cmocka_unit_test(new__malloc_ERRNO),
	cmocka_unit_test(new__malloc_ERRNO_subsequent_ERRNO2),
	{"new__conn_cfg_default_success",
		new__success, setup__conn_req_new, teardown__conn_req_new,
		&prestate_conn_cfg_default},
	{"new__conn_cfg_custom_success",
		new__success, setup__conn_req_new, teardown__conn_req_new,
		&prestate_conn_cfg_custom},
	cmocka_unit_test(NULL)
};

int
main(int argc, char *argv[])
{
	/* prepare prestate - default conn_cfg */
	prestate_init(&prestate_conn_cfg_default, MOCK_CONN_CFG_DEFAULT,
			RPMA_DEFAULT_TIMEOUT_MS, MOCK_CQ_SIZE_DEFAULT);

	/* prepare prestate - custom conn_cfg */
	prestate_init(&prestate_conn_cfg_custom, MOCK_CONN_CFG_CUSTOM,
			MOCK_TIMEOUT_MS_CUSTOM, MOCK_CQ_SIZE_CUSTOM);

	return cmocka_run_group_tests(test_new, NULL, NULL);
}
