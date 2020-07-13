/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2020, Intel Corporation
 */

/*
 * log-test-disabled.c -- unit tests of the log module with logging
 * not enabled at startup
 */

#include <stdlib.h>
#include <syslog.h>
#include <string.h>
#include "cmocka_headers.h"
#include "../../src/common/log_internal.h"
#include "log-test-common.h"

/*
 * openlog -- openlog() mock
 */
void
openlog(const char *__ident, int __option, int __facility)
{
	check_expected(__ident);
	check_expected(__option);
	check_expected(__facility);
}

/*
 * closelog -- closelog() mock
 */
void
closelog(void)
{
	function_called();
}

/*
 * custom_log_function -- use-defined custom log function mock. It is used
 * instead of writing to syslog/stderr.
 */
static void
custom_log_function(int level, const char *file, const int line,
	const char *func, const char *format, va_list args)
{
	check_expected(level);
	check_expected(file);
	check_expected(line);
	check_expected(func);
	check_expected(format);
}

/*
 * setup_without_custom_log_function -- logging setup without user-defined function
 * default log enabling path expected
 */
static int
setup_without_custom_log_function(void **p_custom_log_function)
{
	*p_custom_log_function = NULL;
	expect_string(openlog, __ident, "rpma");
	expect_value(openlog, __option, LOG_PID);
	expect_value(openlog, __facility, LOG_LOCAL7);
	assert_int_equal(0, rpma_log_init(NULL));
	return 0;
}

/*
 * setup_with_custom_log_function -- logging setup with user-defined log function
 * no use of syslog and stderr
 */
static int
setup_with_custom_log_function(void **p_custom_log_function)
{
	*p_custom_log_function = custom_log_function;
	assert_int_equal(0, rpma_log_init(custom_log_function));
	return 0;
}

/*
 * teardown -- logging finit and closelog() called in case of no user-defined
 * log function
 */
static int
teardown(void **p_custom_log_function)
{
	log_function *custom_log_function = *p_custom_log_function;
	if (NULL == custom_log_function)
		expect_function_call(closelog);
	rpma_log_fini();
	return 0;
}

/*
 * test_log_lifecycle -- logging lifecycle - proper sequence of in logging
 * initialization and shutdown
 */
static void
test_log_lifecycle(void **unused)
{
	/*
	 * The whole thing is done by setup_without_custom_log_function()
	 * or setup_with_custom_log_function
	 * and teardown().
	 */
}

/*
 * test_log_to_user_function -- logging via custom user-defined log function
 *
 */
static void
test_log_to_user_function(void **p_logfunction)
{
	for (enum rpma_log_level level = RPMA_LOG_DISABLED;
		level <= RPMA_LOG_LEVEL_DEBUG; level++) {
		expect_value(custom_log_function, level, level);
		expect_string(custom_log_function, file, "file");
		expect_value(custom_log_function, line, 1);
		expect_string(custom_log_function, func, "func");
		expect_string(custom_log_function, format, "%s");
		rpma_log(level, "file", 1, "func", "%s", "msg");
	}
}

int
main(int argc, char *argv[])
{
	const struct CMUnitTest tests[] = {
		/*
		 * lifecycle tests for initialization with and without
		 * custom log function provided
		 */
		cmocka_unit_test_setup_teardown(test_log_lifecycle,
			setup_without_custom_log_function, teardown),
		cmocka_unit_test_setup_teardown(test_log_lifecycle,
			setup_with_custom_log_function, teardown),
		/*
		 * negative tests with custom log function
		 */
		cmocka_unit_test_setup_teardown(test_set_level,
			setup_without_custom_log_function, teardown),
		cmocka_unit_test_setup_teardown(test_set_level,
			setup_with_custom_log_function, teardown),
		cmocka_unit_test_setup_teardown(test_set_level_invalid,
			setup_without_custom_log_function, teardown),
		cmocka_unit_test_setup_teardown(test_set_level_invalid,
			setup_with_custom_log_function, teardown),
		/*
		 * negative test with default log function
		 */
		cmocka_unit_test_setup_teardown(test_set_print_level,
			setup_without_custom_log_function, teardown),
		cmocka_unit_test_setup_teardown(test_set_print_level,
			setup_with_custom_log_function, teardown),
		cmocka_unit_test_setup_teardown(test_set_print_level_invalid,
			setup_without_custom_log_function, teardown),
		cmocka_unit_test_setup_teardown(test_set_print_level_invalid,
			setup_with_custom_log_function, teardown),
		cmocka_unit_test_setup_teardown(test_log_to_user_function,
			setup_with_custom_log_function, teardown),
		/*
		 * negative test with level out of threshold
		 */
		cmocka_unit_test_setup_teardown(test_log_out_of_threshold,
			setup_without_custom_log_function, teardown),
		cmocka_unit_test_setup_teardown(test_log_to_syslog,
			setup_without_custom_log_function, teardown),
		cmocka_unit_test_setup_teardown(test_log_to_syslog_no_file,
			setup_without_custom_log_function, teardown),
	};

	return cmocka_run_group_tests(tests, NULL, NULL);
}
