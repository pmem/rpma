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
 * user_logfunc -- use-defined log function mock
 */
static void
user_logfunc(int level, const char *file, const int line,
	const char *func, const char *format, va_list args)
{
	check_expected(level);
	check_expected(file);
	check_expected(line);
	check_expected(func);
	check_expected(format);
}

/*
 * setup_without_logfunction -- logging setup without user-defined function
 * default log enabling path expected
 */
static int
setup_without_logfunction(void **p_logfunction)
{
	*p_logfunction = NULL;
	expect_string(openlog, __ident, "rpma");
	expect_value(openlog, __option, LOG_PID);
	expect_value(openlog, __facility, LOG_LOCAL7);
	assert_int_equal(0, rpma_log_init(NULL));
	return 0;
}

/*
 * setup_with_logfunction -- logging setup with user-defined log function
 * no use of syslog and stderr
 */
static int
setup_with_logfunction(void **p_logfunction)
{
	*p_logfunction = user_logfunc;
	assert_int_equal(0, rpma_log_init(user_logfunc));
	return 0;
}

/*
 * teardown -- logging finit and closelog() called in case of no user-defined
 * log function
 */
static int
teardown(void **p_logfunction)
{
	log_function *logf = *p_logfunction;
	if (NULL == logf) {
		expect_function_call(closelog);
	}
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
		expect_value(user_logfunc, level, level);
		expect_string(user_logfunc, file, "file");
		expect_value(user_logfunc, line, 1);
		expect_string(user_logfunc, func, "func");
		expect_string(user_logfunc, format, "%s");
		rpma_log(level, "file", 1, "func", "%s", "msg");
	}
}

int
main(int argc, char *argv[])
{
	const struct CMUnitTest tests[] = {
		cmocka_unit_test_setup_teardown(test_log_lifecycle,
			setup_without_logfunction, teardown),
		cmocka_unit_test_setup_teardown(test_log_lifecycle,
			setup_with_logfunction, teardown),

		cmocka_unit_test_setup_teardown(test_set_level,
			setup_without_logfunction, teardown),
		cmocka_unit_test_setup_teardown(test_set_level,
			setup_with_logfunction, teardown),
		cmocka_unit_test_setup_teardown(test_set_level_invalid,
			setup_without_logfunction, teardown),
		cmocka_unit_test_setup_teardown(test_set_level_invalid,
			setup_with_logfunction, teardown),

		cmocka_unit_test_setup_teardown(test_set_print_level,
			setup_without_logfunction, teardown),
		cmocka_unit_test_setup_teardown(test_set_print_level,
			setup_with_logfunction, teardown),
		cmocka_unit_test_setup_teardown(test_set_print_level_invalid,
			setup_without_logfunction, teardown),
		cmocka_unit_test_setup_teardown(test_set_print_level_invalid,
			setup_with_logfunction, teardown),

		cmocka_unit_test_setup_teardown(test_log_to_user_function,
			setup_with_logfunction, teardown),

		cmocka_unit_test_setup_teardown(test_log_out_of_threshold,
			setup_without_logfunction, teardown),
		cmocka_unit_test_setup_teardown(test_log_to_syslog,
			setup_without_logfunction, teardown),
		cmocka_unit_test_setup_teardown(test_log_to_syslog_no_file,
				setup_without_logfunction, teardown),


	};

	return cmocka_run_group_tests(tests, NULL, NULL);
}
