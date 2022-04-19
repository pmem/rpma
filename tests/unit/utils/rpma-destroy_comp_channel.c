// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2022, Intel Corporation */

/*
 * rpma-destroy_comp_channel.c -- a unit test for
 * rpma_ibv_destroy_comp_channel()
 */

#include "cmocka_headers.h"
#include "rpma.h"
#include "test-common.h"
#include "mocks-ibverbs.h"

/*
 * ibv_destroy__ibv_destroy_fail - ibv_destroy_comp_channel return ERRNO
 */
static void
ibv_destroy__ibv_destroy_fail(void **unused)
{
	/* configure mocks */
	will_return(ibv_destroy_comp_channel, MOCK_ERRNO);

	int ret = rpma_ibv_destroy_comp_channel(MOCK_COMP_CHANNEL);

	assert_int_equal(ret, RPMA_E_PROVIDER);
}

/*
 * ibv_destroy__success - happyday scenario
 */
static void
ibv_destroy__success(void **unused)
{
	/* configure mocks */
	will_return(ibv_destroy_comp_channel, MOCK_OK);

	int ret = rpma_ibv_destroy_comp_channel(MOCK_COMP_CHANNEL);

	assert_int_equal(ret, MOCK_OK);
}

int
main(int argc, char *argv[])
{
	const struct CMUnitTest tests[] = {
		/* rpma_utils_conn_event_2str() unit tests */
		cmocka_unit_test(ibv_destroy__ibv_destroy_fail),
		cmocka_unit_test(ibv_destroy__success),
	};

	return cmocka_run_group_tests(tests, NULL, NULL);
}
