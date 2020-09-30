// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020, Intel Corporation */

/*
 * threshold.c -- rpma_log_[get/set]_threshold unit tests
 */

#include <stdlib.h>

#include "cmocka_headers.h"
#include "log_internal.h"
#include "log_default.h"
#include "librpma.h"

void
custom_function(enum rpma_log_level level, const char *file_name,
	const int line_no, const char *function_name,
	const char *message_format, ...)
{
	;
}

/*
 * log_function_failed -- set log function failed
 */
void
log_function_failed(void **unused)
{
	/* configure mocks */
	will_return(__wrap_sync_bool_compare_and_swap_void, 0);
	will_return(__wrap_sync_bool_compare_and_swap_void, 1);

	int ret = rpma_log_set_function(custom_function);
	assert_int_equal(ret, RPMA_E_AGAIN);
	ret = rpma_log_set_function(custom_function);
	assert_int_equal(ret, 0);
	assert_ptr_equal(Rpma_log_function, custom_function);

}

/*
 * log_function_lifecycle -- happy day scenario
 */
void
log_function_lifecycle(void **unused)
{
	/* configure mocks */
	will_return(__wrap_sync_bool_compare_and_swap_void, 1);

	int ret = rpma_log_set_function(custom_function);
	assert_int_equal(ret, 0);
	assert_ptr_equal(Rpma_log_function, custom_function);
}

int
main(int argc, char *argv[])
{
	const struct CMUnitTest tests[] = {
			/* rpma_log_set_function() success test */
			cmocka_unit_test(log_function_lifecycle),

			/* rpma_log_set_function() failed test */
			cmocka_unit_test(log_function_failed),
	};
	return cmocka_run_group_tests(tests, NULL, NULL);
}
