/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2020, Intel Corporation
 */

/*
 * log-test-to-custom-function.c -- unit tests of the log module with custom
 * log function provided in rpma_log_init() function
 */

#include <stdlib.h>
#include <syslog.h>
#include <string.h>
#include "log-test-to-syslog.h"

/*
 * custom_log_function -- use-defined custom log function mock. It is used
 * instead of writing to syslog/stderr.
 */
static void
custom_log_function(rpma_log_level level, const char *file_name,
		const int line_no, const char *function_name,
		const char *format, va_list args)
{
	check_expected(level);
	if (file_name)
		check_expected(file_name);
	check_expected(line_no);
	if (function_name)
		check_expected(function_name);
	check_expected(format);

	char custom_log_function_output [256];
	int ret = vsnprintf(custom_log_function_output,
		sizeof(custom_log_function_output), format, args);
	assert_true(ret > 0);
	if (ret > 0)
		check_expected_ptr(custom_log_function_output);
}

enum rpma_log_input {
	rpma_log_input_non_null,
	rpma_log_input_file_name_null,
	rpma_log_input_file_name_function_name_null
};

/*
 * log__to_user_function -- logging via custom user-defined log function
 */
void
log__to_user_function(void **config_ptr)
{
	enum rpma_log_input *config = (enum rpma_log_input *) *config_ptr;
	/*
	 * disable mocks for clean log shutdown
	 */
	syslog_mock_disable();
	rpma_log_fini();

	/*
	 * enable syslog mocks to monitor if no output is written to syslog()
	 */
	syslog_mock_enable();

	/*
	 * start log with custom function
	 */
	assert_int_equal(0, rpma_log_init(custom_log_function));
	for (rpma_log_level level = RPMA_LOG_DISABLED;
		level <= RPMA_LOG_LEVEL_DEBUG; level++) {
		/*
		 * mock setup
		 */
		expect_value(custom_log_function, level, level);

		if (rpma_log_input_non_null == *config) {
			expect_string(custom_log_function, file_name,
					TEST_FILE_NAME);
		}
		expect_value(custom_log_function, line_no, TEST_LINE_NO);
		if (rpma_log_input_file_name_function_name_null != *config) {
			expect_string(custom_log_function, function_name,
				TEST_FUNCTION_NAME);
		}
		expect_string(custom_log_function, format, "%s");
		expect_string(custom_log_function, custom_log_function_output,
				TEST_MESSAGE);
		/*
		 * run test
		 */
		rpma_log(level,
			rpma_log_input_non_null == *config?TEST_FILE_NAME:NULL,
			TEST_LINE_NO,
			rpma_log_input_file_name_function_name_null != *config?
				TEST_FUNCTION_NAME: NULL,
			"%s", TEST_MESSAGE);
	}
	rpma_log_fini();
}

/*
 * for tests with file name and function name provided
 */
static enum rpma_log_input non_null = rpma_log_input_non_null;
/*
 * for tests with only function name provided
 */
static enum rpma_log_input file_name__null = rpma_log_input_file_name_null;
/*
 * for tests without file name and function name provided
 */
static enum rpma_log_input file_name_function_name_null =
				rpma_log_input_file_name_function_name_null;

const struct CMUnitTest log_test_to_custom_function[] = {
	/*
	 * test with file name and function name provided
	 */
	cmocka_unit_test_prestate(log__to_user_function,
				&non_null),
	/*
	 * test with no file name provided
	 */
	cmocka_unit_test_prestate(log__to_user_function,
				&file_name__null),
	/*
	 * test with no file name and no function name provided
	 */
	cmocka_unit_test_prestate(log__to_user_function,
				&file_name_function_name_null),
	cmocka_unit_test(NULL)
};
