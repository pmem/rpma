/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2020, Intel Corporation
 */

/*
 * log-test.c -- unit tests of the log module
 */

#include <stdlib.h>
#include <syslog.h>
#include <string.h>

#include "../../src/common/log_internal.h"
#include "cmocka_headers.h"
#include "librpma_log.h"
#include "librpma.h"

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

static char syslog_temporary_buffer[256];
/*
 * syslog -- syslog() mock
 */
void
syslog(int __pri, const char *__fmt, ...)
{
	int retval;
	va_list args;
	check_expected(__pri);
	va_start(args, __fmt);
	retval = vsnprintf(syslog_temporary_buffer,
			sizeof(syslog_temporary_buffer), __fmt, args);
	check_expected_ptr(syslog_temporary_buffer);
	va_end(args);
	assert_true(retval > 0);
}

static void user_logfunc(int level, const char *file, const int line,
		const char *func, const char *format, va_list args)
{
	check_expected(level);
	check_expected(file);
	check_expected(line);
	check_expected(func);
	check_expected(format);
}

static int
setup_without_logfunction(void **p_logfunction)
{
	*p_logfunction = NULL;
	expect_string(openlog, __ident, "rpma");
	expect_value(openlog, __option, LOG_PID);
	expect_value(openlog, __facility, LOG_LOCAL7);
	rpma_log_open(NULL);
	return 0;
}

static int
setup_with_logfunction(void **p_logfunction)
{
	*p_logfunction = user_logfunc;
	rpma_log_open(user_logfunc);
	return 0;
}

static int
teardown(void **p_logfunction)
{
	logfunc *logf = *p_logfunction;
	if (NULL == logf) {
		expect_function_call(closelog);
	}
	rpma_log_close();
	return 0;
}

static void
test_open_close_no_logfunction(void **unused)
{

}

static void
test_open_close_logfunction(void **unused)
{

}

static void
test_set_level(void **unused)
{
	enum rpma_log_level level;
	for (level = RPMA_LOG_DISABLED; level <= RPMA_LOG_DEBUG; level ++) {
		assert_int_equal(0, rpma_log_set_level(level));
		assert_int_equal(level, rpma_log_get_level());
	}
}

static void
test_set_level_invalid(void **unused)
{
	enum rpma_log_level level = RPMA_LOG_DEBUG;
	level ++;
	assert_int_equal(RPMA_E_INVAL, rpma_log_set_level(level));
	level = RPMA_LOG_DISABLED;
	level --;
	assert_int_equal(RPMA_E_INVAL, rpma_log_set_level(level));
}

static void
test_set_print_level(void **unused)
{
	enum rpma_log_level level;
	for (level = RPMA_LOG_DISABLED; level <= RPMA_LOG_DEBUG; level ++) {
		assert_int_equal(0, rpma_log_set_print_level(level));
		assert_int_equal(level, rpma_log_get_print_level());
	}
}

static void
test_set_print_level_invalid(void **unused)
{
	enum rpma_log_level level = RPMA_LOG_DEBUG;
	level ++;
	assert_int_equal(RPMA_E_INVAL, rpma_log_set_print_level(level));
	level = RPMA_LOG_DISABLED;
	level --;
	assert_int_equal(RPMA_E_INVAL, rpma_log_set_print_level(level));
}

static void
test_log_to_user_function(void **p_logfunction)
{
	enum rpma_log_level level;
	for (level = RPMA_LOG_DISABLED; level <= RPMA_LOG_DEBUG; level ++) {
		expect_value(user_logfunc, level, level);
		expect_string(user_logfunc, file, "file");
		expect_value(user_logfunc, line, 1);
		expect_string(user_logfunc, func, "func");
		expect_string(user_logfunc, format, "%s");
		rpma_log(level, "file", 1, "func", "%s", "msg");
	}
}

static void
test_log_disabled(void **unused)
{
	enum rpma_log_level level_min;
	for (level_min = RPMA_LOG_DISABLED;
			level_min <= RPMA_LOG_DEBUG; level_min ++) {
		assert_int_equal(0, rpma_log_set_level(level_min));
		assert_int_equal(0, rpma_log_set_print_level(level_min));
		enum rpma_log_level level;
		for (level = level_min + 1; level <= RPMA_LOG_DEBUG; level ++) {
			rpma_log(level, "file", 1, "func", "%s", "msg");
		}
	}

}

static const char *const rpma_level_names[] = {
	[RPMA_LOG_ERROR]	= "ERROR",
	[RPMA_LOG_WARN]		= "WARNING",
	[RPMA_LOG_NOTICE]	= "NOTICE",
	[RPMA_LOG_INFO]		= "INFO",
	[RPMA_LOG_DEBUG]	= "DEBUG",
};

static const int rpma_level_syslog[] = {
	[RPMA_LOG_ERROR]	= LOG_ERR,
	[RPMA_LOG_WARN]		= LOG_WARNING,
	[RPMA_LOG_NOTICE]	= LOG_NOTICE,
	[RPMA_LOG_INFO]		= LOG_INFO,
	[RPMA_LOG_DEBUG]	= LOG_DEBUG,
};

static void
test_log_to_syslog(void **unused)
{
	static char expected_string[256] = "";
	rpma_log_set_print_level(RPMA_LOG_DISABLED);
	rpma_log_set_level(RPMA_LOG_DEBUG);
	enum rpma_log_level level;
	for (level = RPMA_LOG_ERROR; level <= RPMA_LOG_DEBUG; level ++) {
		expect_value(syslog, __pri, rpma_level_syslog[level]);
		expected_string[0] = '\0';
		strcat(expected_string, "file:   1:func: *");
		strcat(expected_string, rpma_level_names[level]);
		strcat(expected_string, "*: msg");
		expect_string(syslog, syslog_temporary_buffer, expected_string);
		rpma_log(level, "file", 1, "func", "%s", "msg");
	}
}

static void
test_log_to_syslog_no_file(void **unused)
{
	rpma_log_set_print_level(RPMA_LOG_DISABLED);
	rpma_log_set_level(RPMA_LOG_DEBUG);
	enum rpma_log_level level;
	for (level = RPMA_LOG_ERROR; level <= RPMA_LOG_DEBUG; level ++) {
		expect_value(syslog, __pri, rpma_level_syslog[level]);
		expect_string(syslog, syslog_temporary_buffer, "msg");
		rpma_log(level, NULL, 0, NULL, "%s", "msg");
	}
}

int
main(int argc, char *argv[])
{
	const struct CMUnitTest tests[] = {
		cmocka_unit_test_setup_teardown(test_open_close_no_logfunction,
			setup_without_logfunction, teardown),
		cmocka_unit_test_setup_teardown(test_open_close_logfunction,
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

		cmocka_unit_test_setup_teardown(test_log_disabled,
			setup_without_logfunction, teardown),
		cmocka_unit_test_setup_teardown(test_log_to_syslog,
			setup_without_logfunction, teardown),
		cmocka_unit_test_setup_teardown(test_log_to_syslog_no_file,
				setup_without_logfunction, teardown),


	};

	int retVal = cmocka_run_group_tests(tests, NULL, NULL);
	return retVal;
}

#if 0
enum rpma_log_level {
	/* All messages will be suppressed. */
	RPMA_LOG_DISABLED = -1,
	RPMA_LOG_ERROR,
	RPMA_LOG_WARN,
	RPMA_LOG_NOTICE,
	RPMA_LOG_INFO,
	RPMA_LOG_DEBUG,
};
#endif
