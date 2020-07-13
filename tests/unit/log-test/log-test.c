// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020, Intel Corporation */

/*
 * log-test-enabled.c -- unit tests of the log module with logging
 * enabled at startup
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
}

/*
 * closelog -- closelog() mock
 */
void
closelog(void)
{
}

/*
 * fprintf -- frpintf() mock
 */
char fprintf_temporary_buffer[1024];
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
	ret = vsnprintf(fprintf_temporary_buffer,
			sizeof(fprintf_temporary_buffer), __format, args);
	va_end(args);

	return ret;
}

void
test_log__log_to_stderr(void **unused)
{
	assert_int_equal(0, rpma_log_syslog_set_threshold(RPMA_LOG_DISABLED));
	assert_int_equal(0, rpma_log_stderr_set_threshold(RPMA_LOG_LEVEL_ERROR));
	expect_function_call(__wrap_fprintf);
	expect_string(__wrap_fprintf, __format, "%s%s%s");
	will_return(__wrap_fprintf, "msg");

	rpma_log(RPMA_LOG_LEVEL_ERROR, "file", 1, "func", "%s", "msg");
}

void
test_log__could_not_start_already_started_log(void **unused)
{
	assert_int_equal(-1, rpma_log_init(NULL));
	rpma_log_fini();
	assert_int_equal(0, rpma_log_init(NULL));
	assert_int_equal(0, rpma_log_syslog_set_threshold(RPMA_LOG_DISABLED));
	assert_int_equal(0, rpma_log_stderr_set_threshold(RPMA_LOG_LEVEL_ERROR));
	expect_function_call(__wrap_fprintf);
	expect_string(__wrap_fprintf, __format, "%s%s%s");
	will_return(__wrap_fprintf, "msg");
	rpma_log(RPMA_LOG_LEVEL_ERROR, "file", 1, "func", "%s", "msg");
	rpma_log_fini();
}

int
main(int argc, char *argv[])
{
	const struct CMUnitTest tests[] = {
		cmocka_unit_test(syslog_set_threshold),
		cmocka_unit_test(syslog_set_threshold__invalid),

		cmocka_unit_test(stderr_set_threshold),
		cmocka_unit_test(stderr_set_threshold__invalid),

		cmocka_unit_test(log__out_of_threshold),
		cmocka_unit_test(log__to_syslog),
		cmocka_unit_test(log__to_syslog_no_file),

		cmocka_unit_test(test_log__log_to_stderr),

		cmocka_unit_test(test_log__could_not_start_already_started_log),
	};

	return cmocka_run_group_tests(tests, NULL, NULL);
}
