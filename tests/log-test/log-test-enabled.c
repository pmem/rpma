// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020, Intel Corporation */

/*
 * log-test-enabled.c -- unit tests of the log module
 * with logging enabled at startup
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
#if 0
	int err = mock_type(int);

	if (err) {
		errno = err;
		return NULL;
	}
	return fvprintf(...);
#endif
}

/*
 * XXX missing tests to check initial logging levels:
 * #ifdef DEBUG
 *	rpma_log_set_level(RPMA_LOG_DEBUG);
 *	rpma_log_set_print_level(RPMA_LOG_WARN);
 * #else
 *	rpma_log_set_level(RPMA_LOG_WARN);
 *	rpma_log_set_print_level(RPMA_LOG_DISABLED);
 * #endif
 */

void
test_log__log_to_stderr(void **unused)
{
	assert_int_equal(0, rpma_log_set_level(RPMA_LOG_DISABLED));
	assert_int_equal(0, rpma_log_stderr_set_level(RPMA_LOG_ERROR));
	expect_function_call(__wrap_fprintf);
	expect_string(__wrap_fprintf, __format, "%s%s%s");
	will_return(__wrap_fprintf, "msg");

	rpma_log(RPMA_LOG_ERROR, "file", 1, "func", "%s", "msg");
}

int
main(int argc, char *argv[])
{
	const struct CMUnitTest tests[] = {
		cmocka_unit_test(test_set_level),
		cmocka_unit_test(test_set_level_invalid),

		cmocka_unit_test(test_set_print_level),
		cmocka_unit_test(test_set_print_level_invalid),

		cmocka_unit_test(test_set_backtrace_level),
		cmocka_unit_test(test_set_backtrace_level_invalid),

		cmocka_unit_test(test_log_out_of_threshold),
		cmocka_unit_test(test_log_to_syslog),
		cmocka_unit_test(test_log_to_syslog_no_file),

		cmocka_unit_test(test_log__log_to_stderr),
	};

	return cmocka_run_group_tests(tests, NULL, NULL);
}
