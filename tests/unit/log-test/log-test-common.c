// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020, Intel Corporation */

/*
 * log-test-common.c -- unit tests of the log module - common tests
 */

#include <syslog.h>
#include <string.h>
#include "cmocka_headers.h"
#include "../../src/common/log_internal.h"

static char syslog_temporary_buffer[1024];

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

/*
 * stderr_set_threshold - is it possible to set all logging to stderr levels?
 */
void
stderr_set_threshold(void **unused)
{
	for (rpma_log_level level = RPMA_LOG_DISABLED;
		level <= RPMA_LOG_LEVEL_DEBUG; level++) {
		assert_int_equal(0, rpma_log_stderr_set_threshold(level));
		assert_int_equal(level, rpma_log_stderr_get_threshold());
	}
}

/*
 * stderr_set_threshold__invalid -- do out of scope logging to stderr levels
 * rejected?
 */
void
stderr_set_threshold__invalid(void **unused)
{
	rpma_log_level level = RPMA_LOG_LEVEL_DEBUG;
	rpma_log_level level_org = rpma_log_stderr_get_threshold();
	level++;
	assert_int_equal(-1, rpma_log_stderr_set_threshold(level));
	assert_int_equal(level_org, rpma_log_stderr_get_threshold());
	level = RPMA_LOG_DISABLED;
	level--;
	assert_int_equal(-1, rpma_log_stderr_set_threshold(level));
	assert_int_equal(level_org, rpma_log_stderr_get_threshold());
}

/*
 * log__out_of_threshold -- no output to stderr and syslog produced
 * for logging level out of threshold
 */
void
log__out_of_threshold(void **unused)
{
	for (rpma_log_level level_min = RPMA_LOG_DISABLED;
			level_min <= RPMA_LOG_LEVEL_DEBUG; level_min++) {
		assert_int_equal(0, rpma_log_syslog_set_threshold(level_min));
		assert_int_equal(0, rpma_log_stderr_set_threshold(level_min));
		for (rpma_log_level level = level_min + 1;
			level <= RPMA_LOG_LEVEL_DEBUG; level++) {
			rpma_log(level, "file", 1, "func", "%s", "msg");
		}
	}
}

static const char *const expected_level_names[] = {
	[RPMA_LOG_LEVEL_FATAL]	= "FATAL",
	[RPMA_LOG_LEVEL_ERROR]	= "ERROR",
	[RPMA_LOG_LEVEL_WARNING] = "WARNING",
	[RPMA_LOG_LEVEL_NOTICE]	= "NOTICE",
	[RPMA_LOG_LEVEL_INFO]	= "INFO",
	[RPMA_LOG_LEVEL_DEBUG]	= "DEBUG",
};

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
	static char expected_string[256] = "";
	rpma_log_stderr_set_threshold(RPMA_LOG_DISABLED);
	rpma_log_syslog_set_threshold(RPMA_LOG_LEVEL_DEBUG);
	for (rpma_log_level level = RPMA_LOG_LEVEL_FATAL;
		level <= RPMA_LOG_LEVEL_DEBUG; level++) {
		expect_value(syslog, __pri, expected_syslog_level[level]);
		expected_string[0] = '\0';
		strcat(expected_string, "file:    1: func: *");
		strcat(expected_string, expected_level_names[level]);
		strcat(expected_string, "*: msg");
		expect_string(syslog, syslog_temporary_buffer, expected_string);
		rpma_log(level, "file", 1, "func", "%s", "msg");
	}
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
		expect_string(syslog, syslog_temporary_buffer, "msg");
		rpma_log(level, NULL, 0, NULL, "%s", "msg");
	}
}
