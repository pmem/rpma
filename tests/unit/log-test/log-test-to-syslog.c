// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020, Intel Corporation */

/* WIP XXX */

/*
 * log-test-to-syslog.c -- unit tests of the log module with the default
 * log function (logging to syslog)
 */

#include "log-test-to-syslog.h"
#include <stdlib.h>
#include <string.h>

/*
 * openlog()/closelog() log are called during library loading/unloading,
 * when mock are not yet configured. We have to enable them before running some
 * tests.
 */
static int Syslog_mock_enabled;

/*
 * syslog_mock_enable() -- enable mock behavior of syslog mocks
 */
void
syslog_mock_enable()
{
	Syslog_mock_enabled = 1;
}

/*
 * syslog_mock_disable() -- disble mock behavior of syslog mocks
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
 * syslog_output -- produced syslog message
 * message shall be composed in a test and provided via expected_string macro.
 * e.g. expect_string(syslog, syslog_output, expected_syslog_message);
 *
 */
static char syslog_output[1024];

/*
 * syslog -- syslog() mock
 */
void
syslog(int __pri, const char *__fmt, ...)
{
	va_list arg_ptr;
	va_start(arg_ptr, __fmt);

	check_expected(__pri);

	int ret = vsnprintf(syslog_output, sizeof(syslog_output),
		__fmt, arg_ptr);
	assert_true(ret > 0);
	check_expected_ptr(syslog_output);

	va_end(arg_ptr);
}

static const int expected_syslog_level[] = {
	[RPMA_LOG_LEVEL_FATAL]	= LOG_CRIT,
	[RPMA_LOG_LEVEL_ERROR]	= LOG_ERR,
	[RPMA_LOG_LEVEL_WARNING] = LOG_WARNING,
	[RPMA_LOG_LEVEL_NOTICE]	= LOG_NOTICE,
	[RPMA_LOG_LEVEL_INFO]	= LOG_INFO,
	[RPMA_LOG_LEVEL_DEBUG]	= LOG_DEBUG,
};

/*
 * ensure that log is enabled & call setup_treshold to backup and setup
 * thresholds according to configuration
 */
int
setup_log(void **config_ptr)
{
	syslog_mock_disable();
	rpma_log_fini();
	assert_int_equal(0, rpma_log_init(NULL));
	syslog_mock_enable();
	// will_return_maybe(__wrap_snprintf, 1);
	return setup_threshold(config_ptr);
}

/*
 * teardown_log() -- close log
 */
int
teardown_log(void **unused)
{
	syslog_mock_disable();
	rpma_log_fini();
	assert_int_equal(0, rpma_log_init(NULL));
	return 0;
}

/*
 * log__to_syslog -- successful logging to syslog with file related
 * information like file name, line number and function name
 */
static void
log__to_syslog(void **unused)
{
	for (rpma_log_level level = RPMA_LOG_LEVEL_FATAL;
	    level <= RPMA_LOG_LEVEL_DEBUG; level++) {

		/* setup syslog() mock, priority and output message */
		expect_value(syslog, __pri, expected_syslog_level[level]);

		char expected_syslog_message[256] = "";
		strcat(expected_syslog_message,
			TEST_FILE_NAME ":    " STR(TEST_LINE_NO) ": " \
			TEST_FUNCTION_NAME ": *");
		strcat(expected_syslog_message, level2string(level));
		strcat(expected_syslog_message, "*: " TEST_MESSAGE);
		expect_string(syslog, syslog_output, expected_syslog_message);

		/* run test */
		rpma_log(level, TEST_FILE_NAME, TEST_LINE_NO,
			TEST_FUNCTION_NAME, "%s", TEST_MESSAGE);
	}
}

/*
 * log__to_syslog_file_name_function_name_NULL -- successful logging to syslog
 * without file name and function name provided
 */
static void
log__to_syslog_file_name_function_name_NULL(void **unused)
{
	for (rpma_log_level level = RPMA_LOG_LEVEL_FATAL;
		level <= RPMA_LOG_LEVEL_DEBUG; level++) {

		expect_value(syslog, __pri, expected_syslog_level[level]);

		char expected_syslog_message[256] = "";
		strcat(expected_syslog_message, "*");
		strcat(expected_syslog_message, level2string(level));
		strcat(expected_syslog_message, "*: " TEST_MESSAGE);
		expect_string(syslog, syslog_output, expected_syslog_message);

		rpma_log(level, NULL, 0, NULL, "%s", TEST_MESSAGE);
	}
}

/*
 * log_to_syslog_no_file_name -- successful logging to syslog without file name
 * provided
 */
static void
log__to_syslog_file_name_NULL(void **unused)
{
	for (rpma_log_level level = RPMA_LOG_LEVEL_FATAL;
		level <= RPMA_LOG_LEVEL_DEBUG; level++) {

		/* file name not given, function name given */
		expect_value(syslog, __pri, expected_syslog_level[level]);
		char expected_syslog_message[256] = "";
		strcat(expected_syslog_message, "*");
		strcat(expected_syslog_message, level2string(level));
		strcat(expected_syslog_message, "*: " TEST_MESSAGE);
		expect_string(syslog, syslog_output, expected_syslog_message);
		rpma_log(level, NULL, TEST_LINE_NO, TEST_FUNCTION_NAME, "%s",
			TEST_MESSAGE);
	}
}

/*
 * log life-cycle scenarios
 */
static void
init_fini__lifecycle(void **unused)
{
	/*
	 * log is opened in setup as it is done during loading of the library
	 *
	 * verify that logging to syslog works as expected
	 */
	log__to_syslog(NULL);

	/* log shall not be reinitialized without closing it first */
	assert_int_equal(-1, rpma_log_init(NULL));

	/* verify that logging to syslog still works as expected */
	log__to_syslog(NULL);

	/* close log */
	expect_function_call(closelog);
	rpma_log_fini();

	/* verify that no output is produced to syslog */
	rpma_log(RPMA_LOG_LEVEL_FATAL, TEST_FILE_NAME, TEST_LINE_NO,
		TEST_FUNCTION_NAME, "%s", TEST_MESSAGE);
	rpma_log(RPMA_LOG_LEVEL_FATAL, NULL, 0, NULL, "%s", TEST_MESSAGE);
};

static void
init_default(void **unused)
{
	syslog_mock_disable();
	rpma_log_fini();
	syslog_mock_enable();

	expect_string(openlog, __ident, "rpma");
	expect_value(openlog, __option, LOG_PID);
	expect_value(openlog, __facility, LOG_LOCAL7);

	rpma_log_init(NULL);
	assert_int_equal(RPMA_LOG_LEVEL_STDERR_DEFAULT,
			rpma_log_stderr_get_threshold());
	assert_int_equal(RPMA_LOG_LEVEL_SYSLOG_DEFAULT,
			rpma_log_syslog_get_threshold());

	expect_function_call(closelog);

	rpma_log_fini();
}

static struct threshold_config config = {
		rpma_log_syslog_set_threshold,
		rpma_log_syslog_get_threshold,
		RPMA_LOG_DISABLED, RPMA_LOG_LEVEL_DEBUG
};

const struct CMUnitTest log_test_to_syslog[] = {
	/* threshold setters/getters tests */
	cmocka_unit_test_prestate_setup_teardown(
		set_threshold__invalid,
		setup_threshold, NULL, &config),

	cmocka_unit_test_prestate_setup_teardown(
		set_threshold__all,
		setup_threshold, NULL, &config),

	/* logging with levels out of threshold */
	cmocka_unit_test_prestate_setup_teardown(
		log__out_of_threshold,
		setup_log,
		teardown_log,
		&config),

	/* logging to syslog */
	cmocka_unit_test_prestate_setup_teardown(
		log__to_syslog,
		setup_log,
		teardown_log,
		&config),

	cmocka_unit_test_prestate_setup_teardown(
		log__to_syslog_file_name_function_name_NULL,
		setup_log,
		teardown_log,
		&config),

	cmocka_unit_test_prestate_setup_teardown(
		log__to_syslog_file_name_NULL,
		setup_log,
		teardown_log,
		&config),

	/* logging to syslog life cycle */
	cmocka_unit_test_prestate_setup_teardown(
		init_fini__lifecycle,
		setup_log,
		teardown_log,
		&config),

	cmocka_unit_test(init_default),

	cmocka_unit_test(NULL)
};
