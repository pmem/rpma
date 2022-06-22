// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020-2022, Intel Corporation */

/*
 * mocks-time.c -- time.h mocks
 */

#include <string.h>
#include <time.h>

#include "cmocka_headers.h"
#include "mocks-time.h"

/*
 * __wrap_clock_gettime -- clock_gettime() mock
 */
int
__wrap_clock_gettime(clockid_t __clock_id, struct timespec *__tp)
{
	assert_int_equal(__clock_id, CLOCK_REALTIME);
	assert_non_null(__tp);
	struct timespec *tp = mock_type(struct timespec *);
	if (NULL == tp)
		return -1;

	memcpy(__tp, tp, sizeof(struct timespec));
	return 0;
}

/*
 * __wrap_localtime_r -- localtime_r() mock
 */
struct tm *
__wrap_localtime_r(const time_t *restrict __timer, struct tm *restrict __result)
{
	assert_non_null(__timer);
	assert_non_null(__result);
	time_t *timer = mock_type(time_t *);
	assert_memory_equal(__timer, timer, sizeof(time_t));
	struct tm *__tm = mock_type(struct tm *);
	if (__tm)
		memcpy(__result, __tm, sizeof(*__result));
	return __tm;
}

size_t
__real_strftime(char *__restrict __s, size_t __maxsize,
	const char *__restrict __format, const struct tm *__restrict __tp);

/*
 * __wrap_strftime -- strftime() mock
 */
size_t
__wrap_strftime(char *__restrict __s, size_t __maxsize,
	const char *__restrict __format, const struct tm *__restrict __tp)
{
	assert_non_null(__s);
	assert_non_null(__format);
	assert_non_null(__tp);

	size_t ret = mock_type(size_t);
	if (ret == MOCK_STRFTIME_ERROR)
		return ret;

	assert_int_equal(ret, MOCK_STRFTIME_SUCCESS);

	ret = __real_strftime(__s, __maxsize, __format, __tp);
	assert_true(ret > 0);

	return ret;
}
