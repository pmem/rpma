/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2020, Intel Corporation
 */

/*
 * init-fini.c -- rpma_log_init/_fini() unit tests
 */

#include "cmocka_headers.h"
#include "log_internal.h"
#include "log_default.h"

/*
 * Default levels of the logging thresholds
 */
#ifdef DEBUG
#define RPMA_LOG_THRESHOLD_DEFAULT RPMA_LOG_LEVEL_DEBUG
#define RPMA_LOG_THRESHOLD_AUX_DEFAULT RPMA_LOG_LEVEL_WARNING
#else
#define RPMA_LOG_THRESHOLD_DEFAULT RPMA_LOG_LEVEL_WARNING
#define RPMA_LOG_THRESHOLD_AUX_DEFAULT RPMA_LOG_DISABLED
#endif

/*
 * init_fini__lifecycle -- happy day scenario
 */
static void
init_fini__lifecycle(void **unused)
{
	/* verify the initial state of the module */
	assert_null(Rpma_log_function);
	assert_int_equal(Rpma_log_threshold[RPMA_LOG_THRESHOLD],
			RPMA_LOG_THRESHOLD_DEFAULT);
	assert_int_equal(Rpma_log_threshold[RPMA_LOG_THRESHOLD_AUX],
			RPMA_LOG_THRESHOLD_AUX_DEFAULT);

	/* configure mocks, run test & verify the results */
	expect_function_call(rpma_log_default_init);
	rpma_log_init();
	assert_ptr_equal(Rpma_log_function, rpma_log_default_function);

	/* configure mocks, run test & verify the results */
	expect_function_call(rpma_log_default_fini);
	rpma_log_fini();
	assert_ptr_equal(Rpma_log_function, NULL);
}

int
main(int argc, char *argv[])
{
	const struct CMUnitTest tests[] = {
		cmocka_unit_test(init_fini__lifecycle),
	};

	return cmocka_run_group_tests(tests, NULL, NULL);
}
