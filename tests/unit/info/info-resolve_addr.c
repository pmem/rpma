// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020, Intel Corporation */
/* Copyright 2021, Fujitsu */

/*
 * info-resolve_addr.c -- unit tests of the info module
 *
 * API covered:
 * - rpma_info_resolve_addr()
 */

#include <stdlib.h>
#include <string.h>

#include "cmocka_headers.h"
#include "test-common.h"
#include "conn_req.h"
#include "info.h"
#include "librpma.h"
#include "info-common.h"
#include "mocks-rdma_cm.h"
#include "mocks-string.h"

#include <infiniband/verbs.h>

/*
 * resolve_addr__resolve_addr_ERRNO -- rdma_resolve_addr() fails
 * with MOCK_ERRNO
 */
static void
resolve_addr__resolve_addr_ERRNO(void **info_state_ptr)
{
	struct info_state *istate = *info_state_ptr;

	/* configure mocks */
	struct rdma_cm_id cmid = {0};
	expect_value(rdma_resolve_addr, id, &cmid);
	expect_value(rdma_resolve_addr, src_addr, MOCK_SRC_ADDR);
	expect_value(rdma_resolve_addr, dst_addr, MOCK_DST_ADDR);
	expect_value(rdma_resolve_addr, timeout_ms, RPMA_DEFAULT_TIMEOUT_MS);
	will_return(rdma_resolve_addr, MOCK_ERRNO);
	expect_value(__wrap_strerror, errnum, MOCK_ERRNO);
	will_return(__wrap_strerror, MOCK_ERROR);

	/* run test */
	int ret = rpma_info_resolve_addr(istate->info, &cmid,
			RPMA_DEFAULT_TIMEOUT_MS);

	/* verify the result */
	assert_int_equal(ret, RPMA_E_PROVIDER);
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
		cmocka_unit_test_setup_teardown(
				resolve_addr__resolve_addr_ERRNO,
				setup__new_active, teardown__delete),
		cmocka_unit_test_setup_teardown(resolve_addr__success,
				setup__new_active, teardown__delete),
	};

	return cmocka_run_group_tests(tests, NULL, NULL);
}
