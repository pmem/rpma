/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2020, Intel Corporation
 */

/*
 * mocks-syslog.c -- syslog mocks
 */

#include "cmocka_headers.h"
#include "mocks-stdio.h"
#include "test-common.h"

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
 * syslog -- syslog() mock
 */
void
syslog(int priority, const char *format, ...)
{
	static char syslog_output[MOCK_BUFF_LEN];

	va_list ap;
	va_start(ap, format);
	int ret = __real_vsnprintf(syslog_output, MOCK_BUFF_LEN, format, ap);
	assert_true(ret > 0);
	va_end(ap);

	int cmd = mock_type(int);
	if (cmd == MOCK_VALIDATE) {
		check_expected(priority);
		check_expected_ptr(syslog_output);
	} else {
		assert_int_equal(cmd, MOCK_PASSTHROUGH);
	}
}
