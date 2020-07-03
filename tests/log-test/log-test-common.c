// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020, Intel Corporation */

/*
 * log-test-common.c -- unit tests of the log module - common tests
 */

#include <syslog.h>
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

void
test_set_level(void **unused)
{
	enum rpma_log_level level;
	for (level = RPMA_LOG_DISABLED; level <= RPMA_LOG_DEBUG; level ++) {
		assert_int_equal(0, rpma_log_set_level(level));
		assert_int_equal(level, rpma_log_get_level());
	}
}

void
test_set_level_invalid(void **unused)
{
	enum rpma_log_level level = RPMA_LOG_DEBUG;
	level ++;
	assert_int_equal(RPMA_E_INVAL, rpma_log_set_level(level));
	level = RPMA_LOG_DISABLED;
	level --;
	assert_int_equal(RPMA_E_INVAL, rpma_log_set_level(level));
}

void
test_set_print_level(void **unused)
{
	enum rpma_log_level level;
	for (level = RPMA_LOG_DISABLED; level <= RPMA_LOG_DEBUG; level ++) {
		assert_int_equal(0, rpma_log_stderr_set_level(level));
		assert_int_equal(level, rpma_log_stderr_get_level());
	}
}

void
test_set_print_level_invalid(void **unused)
{
	enum rpma_log_level level = RPMA_LOG_DEBUG;
	level ++;
	assert_int_equal(RPMA_E_INVAL, rpma_log_stderr_set_level(level));
	level = RPMA_LOG_DISABLED;
	level --;
	assert_int_equal(RPMA_E_INVAL, rpma_log_stderr_set_level(level));
}

void
test_log_out_of_threshold(void **unused)
{
	enum rpma_log_level level_min;
	for (level_min = RPMA_LOG_DISABLED;
			level_min <= RPMA_LOG_DEBUG; level_min ++) {
		assert_int_equal(0, rpma_log_set_level(level_min));
		assert_int_equal(0, rpma_log_stderr_set_level(level_min));
		enum rpma_log_level level;
		for (level = level_min + 1; level <= RPMA_LOG_DEBUG; level ++) {
			rpma_log(level, "file", 1, "func", "%s", "msg");
		}
	}

}

static const char *const rpma_level_names[] = {
	[RPMA_LOG_FATAL]	= "FATAL",
	[RPMA_LOG_ERROR]	= "ERROR",
	[RPMA_LOG_WARN]		= "WARNING",
	[RPMA_LOG_NOTICE]	= "NOTICE",
	[RPMA_LOG_INFO]		= "INFO",
	[RPMA_LOG_DEBUG]	= "DEBUG",
};

static const int rpma_level_syslog[] = {
	[RPMA_LOG_FATAL]	= LOG_CRIT,
	[RPMA_LOG_ERROR]	= LOG_ERR,
	[RPMA_LOG_WARN]		= LOG_WARNING,
	[RPMA_LOG_NOTICE]	= LOG_NOTICE,
	[RPMA_LOG_INFO]		= LOG_INFO,
	[RPMA_LOG_DEBUG]	= LOG_DEBUG,
};

void
test_log_to_syslog(void **unused)
{
	static char expected_string[256] = "";
	rpma_log_stderr_set_level(RPMA_LOG_DISABLED);
	rpma_log_set_level(RPMA_LOG_DEBUG);
	enum rpma_log_level level;
	for (level = RPMA_LOG_DISABLED; level <= RPMA_LOG_DEBUG; level ++) {
		if(level == RPMA_LOG_DISABLED) {
			rpma_log(level, "file", 1, "func", "%s", "msg");
		} else {
			expect_value(syslog, __pri, rpma_level_syslog[level]);
			expected_string[0] = '\0';
			strcat(expected_string, "file:   1:func: *");
			strcat(expected_string, rpma_level_names[level]);
			strcat(expected_string, "*: msg");
			expect_string(syslog, syslog_temporary_buffer, expected_string);
			rpma_log(level, "file", 1, "func", "%s", "msg");
		}
	}
}

void
test_log_to_syslog_no_file(void **unused)
{
	rpma_log_stderr_set_level(RPMA_LOG_DISABLED);
	rpma_log_set_level(RPMA_LOG_DEBUG);
	enum rpma_log_level level;
	for (level = RPMA_LOG_ERROR; level <= RPMA_LOG_DEBUG; level ++) {
		expect_value(syslog, __pri, rpma_level_syslog[level]);
		expect_string(syslog, syslog_temporary_buffer, "msg");
		rpma_log(level, NULL, 0, NULL, "%s", "msg");
	}
}
