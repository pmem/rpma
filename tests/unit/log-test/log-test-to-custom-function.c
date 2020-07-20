/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2020, Intel Corporation
 */

/*
 * log-test-to-custom-function.c -- unit tests of the log module with custom
 * log function set in rpma_log_init() function
 */

#include <stdlib.h>
#include <syslog.h>
#include <string.h>
#include "log-test-to-syslog.h"

/*
 * custom_log_function -- user-defined custom log function mock. It is used
 * instead of the default function that writes to syslog/stderr.
 */
static void
custom_log_function(rpma_log_level level, const char *file_name,
		const int line_no, const char *function_name,
		const char *message_format, va_list args)
{
	check_expected(level);
	check_expected(file_name);
	check_expected(line_no);
	check_expected(function_name);
	check_expected(message_format);
	char *arg = va_arg(args, char *);
	char *message = mock_ptr_type(char *);
	assert_string_equal(arg, message);
}

/*
 * lrpma_log_input -- test condition selection enum
 */
enum rpma_log_input {
	/* file name and function name is given */
	rpma_log_input__no_nulls,
	/* only function name is given */
	rpma_log_input__file_NULL,
	/* file name and function name are NULL */
	rpma_log_input__file_function_NULL
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
	 * enable syslog mocks to monitor there is no calls related to syslog()
	 */
	syslog_mock_enable();

	/*
	 * start log with a custom function
	 */
	assert_int_equal(0, rpma_log_init(custom_log_function));
	for (rpma_log_level level = RPMA_LOG_DISABLED;
		level <= RPMA_LOG_LEVEL_DEBUG; level++) {
		/*
		 * mock setup
		 */
		expect_value(custom_log_function, level, level);

		if (rpma_log_input__no_nulls == *config) {
			expect_string(custom_log_function, file_name,
					TEST_FILE_NAME);
		} else {
			expect_value(custom_log_function, file_name, NULL);
		}
		expect_value(custom_log_function, line_no, TEST_LINE_NO);
		if (rpma_log_input__file_function_NULL != *config) {
			expect_string(custom_log_function, function_name,
				TEST_FUNCTION_NAME);
		} else {
			expect_value(custom_log_function, function_name, NULL);
		}

		expect_string(custom_log_function, message_format, "%s");
		will_return(custom_log_function, TEST_MESSAGE);
		/*
		 * run test
		 */
		rpma_log(level,
			rpma_log_input__no_nulls == *config?TEST_FILE_NAME:NULL,
			TEST_LINE_NO,
			rpma_log_input__file_function_NULL != *config?
				TEST_FUNCTION_NAME: NULL,
			"%s", TEST_MESSAGE);
	}
	rpma_log_fini();

	/*
	 * ensure that custom log function is not call in any situation
	 */
	for (rpma_log_level level = RPMA_LOG_DISABLED;
		level <= RPMA_LOG_LEVEL_DEBUG; level++) {
		/*
		 * run test
		 */
		rpma_log(level,
			rpma_log_input__no_nulls == *config?TEST_FILE_NAME:NULL,
			TEST_LINE_NO,
			rpma_log_input__file_function_NULL != *config?
				TEST_FUNCTION_NAME: NULL,
			"%s", TEST_MESSAGE);
	}
}

/*
 * for tests with file name and function name provided
 */
static enum rpma_log_input non_null = rpma_log_input__no_nulls;
/*
 * for tests with only function name provided
 */
static enum rpma_log_input file_name__null = rpma_log_input__file_NULL;
/*
 * for tests without file name and function name provided
 */
static enum rpma_log_input file_name_function_name_null =
				rpma_log_input__file_function_NULL;

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
