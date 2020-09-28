// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020, Intel Corporation */

/*
 * info-resolve_addr.c -- unit tests of the info module
 *
 * API covered:
 * - rpma_info_resolve_addr()
 */

#include <stdlib.h>
#include <string.h>

#include "cmocka_headers.h"
#include "conn_req.h"
#include "info.h"
#include "librpma.h"
#include "info-common.h"
#include "mocks-rdma_cm.h"
#include "rpma_err.h"

#include <infiniband/verbs.h>

/*
 * resolve_addr__id_NULL -- NULL id is invalid
 */
static void
resolve_addr__id_NULL(void **info_state_ptr)
{
	struct info_state *istate = *info_state_ptr;

	/* run test */
	int ret = rpma_info_resolve_addr(istate->info, NULL,
			RPMA_DEFAULT_TIMEOUT_MS);

	/* verify the result */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * resolve_addr__info_NULL -- NULL info is invalid
 */
static void
resolve_addr__info_NULL(void **unused)
{
	/* run test */
	struct rdma_cm_id cmid = {0};
	int ret = rpma_info_resolve_addr(NULL, &cmid, RPMA_DEFAULT_TIMEOUT_MS);

	/* verify the result */
	assert_int_equal(ret, RPMA_E_INVAL);
	assert_int_equal(memcmp(&cmid, &Cmid_zero, sizeof(cmid)), 0);
}

/*
 * resolve_addr__id_info_NULL -- NULL id and info are invalid
 */
static void
resolve_addr__id_info_NULL(void **unused)
{
	/* run test */
	int ret = rpma_info_resolve_addr(NULL, NULL, RPMA_DEFAULT_TIMEOUT_MS);

	/* verify the result */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * resolve_addr__resolve_addr_EAGAIN -- rdma_resolve_addr() fails
 * with EAGAIN
 */
static void
resolve_addr__resolve_addr_EAGAIN(void **info_state_ptr)
{
	struct info_state *istate = *info_state_ptr;

	/* configure mocks */
	struct rdma_cm_id cmid = {0};
	expect_value(rdma_resolve_addr, id, &cmid);
	expect_value(rdma_resolve_addr, src_addr, MOCK_SRC_ADDR);
	expect_value(rdma_resolve_addr, dst_addr, MOCK_DST_ADDR);
	expect_value(rdma_resolve_addr, timeout_ms, RPMA_DEFAULT_TIMEOUT_MS);
	will_return(rdma_resolve_addr, EAGAIN);

	/* run test */
	int ret = rpma_info_resolve_addr(istate->info, &cmid,
			RPMA_DEFAULT_TIMEOUT_MS);

	/* verify the result */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_int_equal(rpma_err_get_provider_error(), EAGAIN);
	assert_int_equal(memcmp(&cmid, &Cmid_zero, sizeof(cmid)), 0);
}

/*
 * resolve_addr__success -- happy day scenario
 */
static void
resolve_addr__success(void **info_state_ptr)
{
	struct info_state *istate = *info_state_ptr;

	/* configure mocks */
	struct rdma_cm_id cmid = {0};
	expect_value(rdma_resolve_addr, id, &cmid);
	expect_value(rdma_resolve_addr, src_addr, MOCK_SRC_ADDR);
	expect_value(rdma_resolve_addr, dst_addr, MOCK_DST_ADDR);
	expect_value(rdma_resolve_addr, timeout_ms, RPMA_DEFAULT_TIMEOUT_MS);
	will_return(rdma_resolve_addr, MOCK_OK);

	/* run test */
	int ret = rpma_info_resolve_addr(istate->info, &cmid,
			RPMA_DEFAULT_TIMEOUT_MS);

	/* verify the result */
	assert_int_equal(ret, MOCK_OK);
	assert_int_equal(memcmp(&cmid, &Cmid_zero, sizeof(cmid)), 0);
}

int
main(int argc, char *argv[])
{
	const struct CMUnitTest tests[] = {
		/* rpma_info_resolve_addr() unit tests */
		cmocka_unit_test_setup_teardown(resolve_addr__id_NULL,
				setup__new_active, teardown__delete),
		cmocka_unit_test(resolve_addr__info_NULL),
		cmocka_unit_test(resolve_addr__id_info_NULL),
		cmocka_unit_test_setup_teardown(
				resolve_addr__resolve_addr_EAGAIN,
				setup__new_active, teardown__delete),
		cmocka_unit_test_setup_teardown(resolve_addr__success,
				setup__new_active, teardown__delete),
	};

	return cmocka_run_group_tests(tests, NULL, NULL);
}
