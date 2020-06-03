// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020, Intel Corporation */

/*
 * log-test-common.c -- unit tests common module. Mock and common tests
 * implementation.
 *
 */

#include <syslog.h>
#include <string.h>
#include "cmocka_headers.h"
#include "../../src/common/log_internal.h"
#include "log-test-common.h"

/*
 * fprintf -- frpintf() mock
 */
char fprintf_output[1024];
int
__wrap_fprintf(FILE *__restrict __stream,
		    const char *__restrict __format, ...)
{
	int ret;
	va_list args;
	va_start(args, __format);
	assert_ptr_equal(__stream, stderr);
	check_expected(__format);
	char *msg, *expected_msg = mock_ptr_type(char *);
	va_arg(args, char *); /* skip timestamp */
	va_arg(args, char *); /* skip prefix */
	msg = va_arg(args, char *);

	assert_string_equal(msg, expected_msg);
	function_called();
	va_end(args);
	va_start(args, __format);
	ret = vsnprintf(fprintf_output,
			sizeof(fprintf_output), __format, args);
	va_end(args);

	return ret;
}

/*
 * setup_default_threshold - setup and verify that default threshold
 * are accepted
 */
int
setup_default_threshold(void **unused)
{
	assert_int_equal(0,
		rpma_log_syslog_set_threshold(RPMA_LOG_LEVEL_SYSLOG_DEFAULT));
	assert_int_equal(RPMA_LOG_LEVEL_SYSLOG_DEFAULT,
		rpma_log_syslog_get_threshold());
	assert_int_equal(0,
		rpma_log_stderr_set_threshold(RPMA_LOG_LEVEL_STDERR_DEFAULT));
	assert_int_equal(RPMA_LOG_LEVEL_STDERR_DEFAULT,
		rpma_log_stderr_get_threshold());
	return 0;
}

void
test_setup_default_treshold(void  **unused) {
	;
}

const struct CMUnitTest tests_log_common[] = {
	cmocka_unit_test_setup(test_setup_default_treshold,
		setup_default_threshold),
	cmocka_unit_test(NULL)
};
