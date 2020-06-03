// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020, Intel Corporation */

/*
 * log-test-default.c -- unit tests of the log module with the default
 * log function (logging to stderr and syslog)
 */

#include <stdlib.h>
#include <string.h>
#include "cmocka_headers.h"
#include "common/log_internal.h"
#include "log-test-to-syslog.h"


/*
 * openlog()/closelog() log are called during library loading/unloading,
 * when mock are not yet configured. We have to enable them before running some
 * tests.
 */
static int Syslog_mock_enabled;

/*
 * syslog_mock_enable() - enable mock behavior of syslog mocks
 */
void
syslog_mock_enable()
{
	Syslog_mock_enabled = 1;
}

/*
 * syslog_mock_disable() - disble mock behavior of syslog mocks
 */
void
syslog_mock_disable()
{
	Syslog_mock_enabled = 0;
}

/*
 * openlog -- openlog() mock
 */
void
openlog(const char *__ident, int __option, int __facility)
{
	if (Syslog_mock_enabled) {
		check_expected(__ident);
		check_expected(__option);
		check_expected(__facility);
	}
}

/*
 * closelog -- closelog() mock
 */
void
closelog(void)
{
	if (Syslog_mock_enabled)
		function_called();
}


/*
 * syslog_message - produced syslog mesage
 * message shall be composed in a test and provided via expected_string macro.
 * e.g. expect_string(syslog, syslog_message, expected_syslog_message);
 *
 */
static char syslog_message[1024];

/*
 * syslog -- syslog() mock
 */
void
syslog(int __pri, const char *__fmt, ...)
{
	va_list arg_ptr;
	va_start(arg_ptr, __fmt);

	check_expected(__pri);

	int retval = vsnprintf(syslog_message, sizeof(syslog_message),
		__fmt, arg_ptr);
	assert_true(retval > 0);
	check_expected_ptr(syslog_message);

	va_end(arg_ptr);
}

/*
 * syslog_set_threshold - is it possible to set all logging levels?
 */
void
syslog_set_threshold(void **unused)
{
	for (rpma_log_level level = RPMA_LOG_DISABLED;
		level <= RPMA_LOG_LEVEL_DEBUG; level++) {
		assert_int_equal(0, rpma_log_syslog_set_threshold(level));
		assert_int_equal(level, rpma_log_syslog_get_threshold());
	}
}

/*
 * syslog_set_threshold__invalid - do out of scope logging levels rejected?
 */
void
syslog_set_threshold__invalid(void **unused)
{
	rpma_log_level level = RPMA_LOG_LEVEL_DEBUG;
	rpma_log_level level_org = rpma_log_syslog_get_threshold();
	level++;
	assert_int_equal(-1, rpma_log_syslog_set_threshold(level));
	assert_int_equal(level_org, rpma_log_syslog_get_threshold());

	level = RPMA_LOG_DISABLED;
	level--;
	assert_int_equal(-1, rpma_log_syslog_set_threshold(level));
	assert_int_equal(level_org, rpma_log_syslog_get_threshold());
}


static const int expected_syslog_level[] = {
	[RPMA_LOG_LEVEL_FATAL]	= LOG_CRIT,
	[RPMA_LOG_LEVEL_ERROR]	= LOG_ERR,
	[RPMA_LOG_LEVEL_WARNING]	= LOG_WARNING,
	[RPMA_LOG_LEVEL_NOTICE]	= LOG_NOTICE,
	[RPMA_LOG_LEVEL_INFO]	= LOG_INFO,
	[RPMA_LOG_LEVEL_DEBUG]	= LOG_DEBUG,
};

/*
 * log__to_syslog -- successful logging to syslog with file related
 * information like file name, line number and function name
 */
void
log__to_syslog(void **unused)
{
	syslog_mock_enable();
	rpma_log_stderr_set_threshold(RPMA_LOG_DISABLED);
	rpma_log_syslog_set_threshold(RPMA_LOG_LEVEL_DEBUG);
	char expected_syslog_message[256] = "";
	for (rpma_log_level level = RPMA_LOG_LEVEL_FATAL;
		level <= RPMA_LOG_LEVEL_DEBUG; level++) {
		expect_value(syslog, __pri, expected_syslog_level[level]);
		expected_syslog_message[0] = '\0';
		strcat(expected_syslog_message,
			TEST_FILE_NAME ":    " STR(TEST_LINE_NO) ": " \
			TEST_FUNCTION_NAME ": *");
		strcat(expected_syslog_message, expected_level_names[level]);
		strcat(expected_syslog_message, "*: " TEST_MESSAGE);
		expect_string(syslog, syslog_message, expected_syslog_message);
		rpma_log(level, TEST_FILE_NAME, TEST_LINE_NO,
			TEST_FUNCTION_NAME, "%s", TEST_MESSAGE);
	}
	syslog_mock_disable();
}

/*
 * log__to_syslog -- successful logging to syslog without file related
 * information
 */
void
log__to_syslog_no_file(void **unused)
{
	rpma_log_stderr_set_threshold(RPMA_LOG_DISABLED);
	rpma_log_syslog_set_threshold(RPMA_LOG_LEVEL_DEBUG);
	for (rpma_log_level level = RPMA_LOG_LEVEL_ERROR;
		level <= RPMA_LOG_LEVEL_DEBUG; level++) {
		expect_value(syslog, __pri, expected_syslog_level[level]);
		expect_string(syslog, syslog_message, TEST_MESSAGE);
		rpma_log(level, NULL, 0, NULL, "%s", TEST_MESSAGE);
	}
}

/*
 * log__to_syslog_out_of_threshold -- no output to syslog produced for logging
 * level out of threshold
 */
void
log__to_syslog_out_of_threshold(void **unused)
{

	assert_int_equal(0, rpma_log_stderr_set_threshold(RPMA_LOG_DISABLED));
	for (rpma_log_level level_min = RPMA_LOG_LEVEL_FATAL;
	    level_min <= RPMA_LOG_LEVEL_DEBUG; level_min++) {
		assert_int_equal(0, rpma_log_syslog_set_threshold(level_min));
		for (rpma_log_level level = level_min + 1;
			level <= RPMA_LOG_LEVEL_DEBUG; level++) {
			rpma_log(level, TEST_FILE_NAME, TEST_LINE_NO,
					TEST_FUNCTION_NAME, "%s", TEST_MESSAGE);
		}
	}
}

void
test_log__could_not_start_already_started_log(void **unused)
{
	syslog_mock_disable();
	rpma_log_fini();
	// openlog("rpma", LOG_PID, LOG_LOCAL7);
	assert_int_equal(0, rpma_log_init(NULL));
//	expect_function_call(openlog);
	syslog_mock_enable();
	assert_int_equal(-1, rpma_log_init(NULL));
	expect_function_call(closelog);
	rpma_log_fini();
#if 0
	assert_int_equal(0, rpma_log_init(NULL));
	assert_int_equal(0, rpma_log_syslog_set_threshold(RPMA_LOG_DISABLED));
	assert_int_equal(0,
		rpma_log_stderr_set_threshold(RPMA_LOG_LEVEL_ERROR));
	expect_function_call(__wrap_fprintf);
	expect_string(__wrap_fprintf, __format, "%s%s%s");
	will_return(__wrap_fprintf, TEST_MESSAGE);
	rpma_log(RPMA_LOG_LEVEL_ERROR, TEST_FILE_NAME, 1, TEST_FUNCTION_NAME,, "%s", TEST_MESSAGE);
	rpma_log_fini();
#endif
	syslog_mock_disable();

};

const struct CMUnitTest tests_log_to_syslog[] = {

/*
 * threshold setters/getters tests
 */
	cmocka_unit_test_setup(syslog_set_threshold__invalid,
		setup_default_threshold),
	cmocka_unit_test_setup(syslog_set_threshold,
		setup_default_threshold),
	cmocka_unit_test_setup(log__to_syslog,
		setup_default_threshold),

/*
 * logging with levels out of threshold
 */
	cmocka_unit_test_setup(log__to_syslog_out_of_threshold,
		setup_default_threshold),

	cmocka_unit_test(test_log__could_not_start_already_started_log),

	cmocka_unit_test(NULL)
};
