// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020, Intel Corporation */
/* Copyright 2021, Fujitsu */

/*
 * info-bind_addr.c -- unit tests of the info module
 *
 * API covered:
 * - rpma_info_bind_addr()
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
 * bind_addr__id_NULL -- NULL id is invalid
 */
static void
bind_addr__id_NULL(void **info_state_ptr)
{
	struct info_state *istate = *info_state_ptr;

	/* run test */
	int ret = rpma_info_bind_addr(istate->info, NULL);

	/* verify the result */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * bind_addr__info_NULL -- NULL info is invalid
 */
static void
bind_addr__info_NULL(void **unused)
{
	/* run test */
	struct rdma_cm_id cmid = {0};
	int ret = rpma_info_bind_addr(NULL, &cmid);

	/* verify the result */
	assert_int_equal(ret, RPMA_E_INVAL);
	assert_int_equal(memcmp(&cmid, &Cmid_zero, sizeof(cmid)), 0);
}

/*
 * bind_addr__id_info_NULL -- NULL id and info are invalid
 */
static void
bind_addr__id_info_NULL(void **unused)
{
	/* run test */
	int ret = rpma_info_bind_addr(NULL, NULL);

	/* verify the result */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * bind_addr__bind_addr_ERRNO -- rpma_info_bind_addr() fails
 * with MOCK_ERRNO
 */
static void
bind_addr__bind_addr_ERRNO(void **info_state_ptr)
{
	struct info_state *istate = *info_state_ptr;

	/* configure mocks */
	struct rdma_cm_id cmid = {0};
	expect_value(rdma_bind_addr, id, &cmid);
	expect_value(rdma_bind_addr, addr, MOCK_SRC_ADDR);
	will_return(rdma_bind_addr, MOCK_ERRNO);
	expect_value(__wrap_strerror, errnum, MOCK_ERRNO);
	will_return(__wrap_strerror, MOCK_ERROR);

	/* run test */
	int ret = rpma_info_bind_addr(istate->info, &cmid);

	/* verify the result */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_int_equal(memcmp(&cmid, &Cmid_zero, sizeof(cmid)), 0);
}

/*
 * bind_addr__success -- happy day scenario
 */
static void
bind_addr__success(void **info_state_ptr)
{
	struct info_state *istate = *info_state_ptr;

	/* configure mocks */
	struct rdma_cm_id cmid = {0};
	expect_value(rdma_bind_addr, id, &cmid);
	expect_value(rdma_bind_addr, addr, MOCK_SRC_ADDR);
	will_return(rdma_bind_addr, MOCK_OK);

	/* run test */
	int ret = rpma_info_bind_addr(istate->info, &cmid);

	/* verify the result */
	assert_int_equal(ret, MOCK_OK);
	assert_int_equal(memcmp(&cmid, &Cmid_zero, sizeof(cmid)), 0);
}

int
main(int argc, char *argv[])
{
	const struct CMUnitTest tests[] = {
		/* rpma_info_bind_addr() unit tests */
		cmocka_unit_test_setup_teardown(bind_addr__id_NULL,
				setup__new_passive, teardown__delete),
		cmocka_unit_test(bind_addr__info_NULL),
		cmocka_unit_test(bind_addr__id_info_NULL),
		cmocka_unit_test_setup_teardown(bind_addr__bind_addr_ERRNO,
				setup__new_passive, teardown__delete),
		cmocka_unit_test_setup_teardown(bind_addr__success,
				setup__new_passive, teardown__delete),
	};

	return cmocka_run_group_tests(tests, NULL, NULL);
}
