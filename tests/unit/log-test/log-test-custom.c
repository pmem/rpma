/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2020, Intel Corporation
 */

/*
 * log-test-custom.c -- unit tests of the log module with custom log function
 * provided in rpma_log_init() function
 */

#include <stdlib.h>
#include <syslog.h>
#include <string.h>
#include "log-test-common.h"


/*
 * custom_log_function -- use-defined custom log function mock. It is used
 * instead of writing to syslog/stderr.
 */
static void
custom_log_function(rpma_log_level level, const char *file, const int line,
	const char *func, const char *format, va_list args)
{
	check_expected(level);
	check_expected(file);
	check_expected(line);
	check_expected(func);
	check_expected(format);
}

#if 0
/*
 * setup_with_custom_log_function -- logging setup with user-defined log
 * function - no use of syslog and stderr
 */
int
setup_with_custom_log_function(void **p_custom_log_function)
{
	*p_custom_log_function = custom_log_function;

	assert_int_equal(0, rpma_log_init(custom_log_function));
	return 0;
}
#endif

/*
 * log__to_user_function -- logging via custom user-defined log function
 */
void
log__to_user_function(void **unused)
{
	rpma_log_fini();
	assert_int_equal(0, rpma_log_init(custom_log_function));
	for (rpma_log_level level = RPMA_LOG_DISABLED;
		level <= RPMA_LOG_LEVEL_DEBUG; level++) {
		expect_value(custom_log_function, level, level);
		expect_string(custom_log_function, file, TEST_FILE_NAME);
		expect_value(custom_log_function, line, TEST_LINE_NO);
		expect_string(custom_log_function, func, TEST_FUNCTION_NAME);
		expect_string(custom_log_function, format, "%s");
		rpma_log(level, TEST_FILE_NAME, TEST_LINE_NO,
			TEST_FUNCTION_NAME, "%s", TEST_MESSAGE);
	}
	rpma_log_fini();
	assert_int_equal(0, NULL);
}

const struct CMUnitTest tests_custom_function[] = {
	cmocka_unit_test(log__to_user_function),
	cmocka_unit_test(NULL)
};
