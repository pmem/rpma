// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2022, Intel Corporation */

/*
 * rpma-create_comp_channel.c -- a unit test for rpma_ibv_create_comp_channel()
 */

#include "cmocka_headers.h"
#include "rpma.h"
#include "test-common.h"
#include "mocks-ibverbs.h"

/*
 * ibv_create__channel_ptr_NULL - channel_ptr NULL is invalid
 */
static void
ibv_create__channel_ptr_NULL(void **unused)
{
	/* configure mocks */
	will_return(ibv_create_comp_channel, NULL);
	will_return(ibv_create_comp_channel, MOCK_ERRNO);

	struct ibv_comp_channel *channel_ptr;
	int ret = rpma_ibv_create_comp_channel(MOCK_VERBS, &channel_ptr);

	assert_null(channel_ptr);
	assert_int_equal(ret, RPMA_E_PROVIDER);
}

/*
 * ibv_create__success - happyday scenario
 */
static void
ibv_create__success(void **unused)
{
	/* configure mocks */
	will_return(ibv_create_comp_channel, MOCK_VERBS);

	struct ibv_comp_channel *channel_ptr;
	int ret = rpma_ibv_create_comp_channel(MOCK_VERBS, &channel_ptr);

	assert_ptr_equal(channel_ptr, MOCK_VERBS);
	assert_int_equal(ret, MOCK_OK);
}

int
main(int argc, char *argv[])
{
	const struct CMUnitTest tests[] = {
		/* rpma_utils_conn_event_2str() unit tests */
		cmocka_unit_test(ibv_create__channel_ptr_NULL),
		cmocka_unit_test(ibv_create__success),
	};

	return cmocka_run_group_tests(tests, NULL, NULL);
}
