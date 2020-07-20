/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2020, Intel Corporation
 */

/*
 * mocks-stdio.c -- stdio mocks
 */

#include "cmocka_headers.h"
#include "mocks-stdio.h"
#include "test-common.h"

/*
 * __wrap_vsnprintf -- vsnprintf() mock
 */
int
__wrap_vsnprintf(char *str, size_t size, const char *format, va_list ap)
{
	int ret = mock_type(int);
	if (ret < 0)
		return ret;

	ret = __real_vsnprintf(str, size, format, ap);
	assert_true(ret > 0);

	return ret;
}

/*
 * __wrap_snprintf -- snprintf() mock
 */
int
__wrap_snprintf(char *str, size_t size, const char *format, ...)
{
	int ret = mock_type(int);
	if (ret < 0)
		return ret;

	va_list ap;
	va_start(ap, format);
	ret = __real_vsnprintf(str, size, format, ap);
	assert_true(ret > 0);
	va_end(ap);

	return ret;
}

/*
 * __wrap_fprintf -- fprintf() mock
 */
int
__wrap_fprintf(FILE *stream, const char *format, ...)
{
	static char fprintf_output[MOCK_BUFF_LEN];

	assert_ptr_equal(stream, stderr);

	va_list ap;
	va_start(ap, format);
	int ret = __real_vsnprintf(fprintf_output, MOCK_BUFF_LEN, format, ap);
	assert_true(ret > 0);
	va_end(ap);

	int cmd = mock_type(int);
	if (cmd == MOCK_STDIO_ERROR)
		return -1;

	if (cmd == MOCK_VALIDATE)
		check_expected_ptr(fprintf_output);
	else
		assert_int_equal(cmd, MOCK_PASSTHROUGH);

	return ret;
}
