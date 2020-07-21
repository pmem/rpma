// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020, Intel Corporation */

/* XXX WIP */

/*
 * log-test-default.c -- unit tests of the log module with the default
 * log function (logging to stderr and syslog)
 */

#include <stdlib.h>
#include <string.h>
#include "time.h"
#include "log-test-to-syslog.h" /* for syslog mock enabling/disabling */

/*
 * fprintf -- frprintf() mock -- dedicated for stderr
 */
static char fprintf_output[1024];

int
__wrap_fprintf(FILE *__restrict __stream, const char *__restrict __format, ...)
{
	/*
	 * we only handle fprintf to stderr
	 */
	assert_ptr_equal(__stream, stderr);

	va_list args;
	va_start(args, __format);

	int ret = vsnprintf(fprintf_output,
			sizeof(fprintf_output), __format, args);
	assert_true(ret > 0);
	if (ret > 0)
		check_expected_ptr(fprintf_output);

	va_end(args);

	return ret;
}

/*
 * snprintf -- snprintf() mock
 */
int
__wrap_snprintf(char *__restrict __s, size_t __maxlen,
		const char *__restrict __format, ...)
{
	// int ret = mock_type(int);
	// if (ret < 0)
	//	return ret;

	va_list args;
	va_start(args, __format);

	int ret = vsnprintf(__s, __maxlen, __format, args);
	assert_true(ret > 0);
//	if (ret > 0)
//		check_expected_ptr(snprintf_output);

	va_end(args);

	return ret;

}

#define TEST_TIME_OF_DAY {00, 00, 00, 1, 0, 70, 0, 365, 0}
#define TEST_TIME_OF_DAY_STR "1970-01-01 00:00:00"
#define TEST_TIME_STR "[" TEST_TIME_OF_DAY_STR ".000000] "
#define TEST_TIME_ERROR_STR "[time error] "

/*
 * __wrap_clock_gettime() -- clock_gettime() mock
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
 * __wrap_localtime() -- localtime() mock
 */
struct tm *
__wrap_localtime(const time_t *__timer)
{
	assert_non_null(__timer);
	time_t *timer = mock_type(time_t *);
	assert_memory_equal(__timer, timer, sizeof(timer_t));

	struct tm *tm = mock_type(struct tm *);
	return tm;
}

/*
 * __wrap_strftime() -- strftime mock
 */
size_t
__wrap_strftime(char *__restrict __s, size_t __maxsize,
	const char *__restrict __format, const struct tm *__restrict __tp)
{
	assert_non_null(__s);
	assert_non_null(__format);
	/* check that format string is not empty */
	assert_int_not_equal(*__format, '\0');

	assert_non_null(__tp);

	char *s = mock_type(char *);
	if (!s)
		return 0;

	/* check expected input time */
	struct tm *tp = mock_type(struct tm *);
	assert_memory_equal(__tp, tp, sizeof(struct tm));

	/* check enough space in result buffer */
	assert_in_range(__maxsize, strlen(s) + 1, 1024);

	strcpy(__s, s);
	return strlen(s);
}

typedef struct {
	const int clock_gettime_error;
	const int localtime_error;
	const int strftime_error;
	struct threshold_config *tresholds;
} mock_config;

/*
 * setup_log_stderr() -- call setup_log_syslog() with proper parameters
 */
int
setup_log_stderr(void **config_ptr)
{
	mock_config *config = (mock_config *)*config_ptr;
	return setup_log((void **)&(config->tresholds));
}

/*
 * teardown_log_stderr() -- call setup_log_syslog() with proper parameters
 */
int
teardown_log_stderr(void **config_ptr)
{
	mock_config *config = (mock_config *)*config_ptr;
	return teardown_log((void **)&(config->tresholds));
}
/*
 * setup time related mocks
 */
#define TIME_MOCKS_INIT(x) \
	if (x->clock_gettime_error) { \
		will_return(__wrap_clock_gettime, NULL); \
	} else if (x->localtime_error) { \
		will_return(__wrap_clock_gettime, &__tp); \
		will_return(__wrap_localtime, &__tp); \
		will_return(__wrap_localtime, NULL); \
	} else if (x->strftime_error) { \
		will_return(__wrap_clock_gettime, &__tp); \
		will_return(__wrap_localtime, &__tp); \
		will_return(__wrap_localtime, &tm); \
		will_return(__wrap_strftime, NULL); \
	} else { \
		will_return(__wrap_clock_gettime, &__tp); \
		will_return(__wrap_localtime, &__tp); \
		will_return(__wrap_localtime, &tm); \
		will_return(__wrap_strftime, strftime_return); \
		will_return(__wrap_strftime, &tm); \
	}

/*
 * log__to_stderr -- successful logging to stderr with file related
 * information like file name, line number and function name
 */
static void
log__to_stderr(void **config_ptr)
{
	mock_config *config = (mock_config *)*config_ptr;
	struct timespec __tp = {0};
	struct tm tm = TEST_TIME_OF_DAY;
	char *strftime_return = TEST_TIME_OF_DAY_STR;

	for (rpma_log_level level = RPMA_LOG_LEVEL_FATAL;
	    level <= RPMA_LOG_LEVEL_DEBUG; level++) {
		/* setup time-related mock */
		TIME_MOCKS_INIT(config);

		/* setup frprintf mock */
		char expected_fprintf_output[256] = "";
		strcat(expected_fprintf_output, config->clock_gettime_error ||
			config->localtime_error || config->strftime_error ?
			TEST_TIME_ERROR_STR : TEST_TIME_STR);
		strcat(expected_fprintf_output, TEST_FILE_NAME ":    " \
			STR(TEST_LINE_NO) ": " TEST_FUNCTION_NAME ": *");
		strcat(expected_fprintf_output, level2string(level));
		strcat(expected_fprintf_output, "*: " TEST_MESSAGE);
		expect_string(__wrap_fprintf, fprintf_output,
				expected_fprintf_output);

		/* run test */
		rpma_log(level, TEST_FILE_NAME, TEST_LINE_NO,
			TEST_FUNCTION_NAME, "%s", TEST_MESSAGE);
	}
}

/*
 * log__to_stderr_file_name_function_name_NULL -- successful logging to stderr
 * without file name and function name provided
 */
static void
log__to_stderr_file_name_function_name_NULL(void **config_ptr)
{
	mock_config *config = (mock_config *)*config_ptr;
	struct timespec __tp = {0};
	struct tm tm = TEST_TIME_OF_DAY;
	char *strftime_return = TEST_TIME_OF_DAY_STR;

	for (rpma_log_level level = RPMA_LOG_LEVEL_FATAL;
		level <= RPMA_LOG_LEVEL_DEBUG; level++) {

		/* setup time-related mock */
		TIME_MOCKS_INIT(config);

		/* setup frprintf mock */
		char expected_fprintf_output[256] = "";
		strcat(expected_fprintf_output, config->clock_gettime_error ||
			config->localtime_error || config->strftime_error ?
			TEST_TIME_ERROR_STR "*": TEST_TIME_STR "*");
		strcat(expected_fprintf_output, level2string(level));
		strcat(expected_fprintf_output, "*: " TEST_MESSAGE);
		expect_string(__wrap_fprintf, fprintf_output,
				expected_fprintf_output);
		rpma_log(level, NULL, 0, NULL, "%s", TEST_MESSAGE);
	}
}

/*
 * log__to_stderr_file_name_NULL -- successful logging to stderr without file
 * name provided
 */
static void
log__to_stderr_file_name_NULL(void **config_ptr)
{
	mock_config *config = (mock_config *)*config_ptr;
	struct timespec __tp = {0};
	struct tm tm = TEST_TIME_OF_DAY;
	char *strftime_return = TEST_TIME_OF_DAY_STR;

	for (rpma_log_level level = RPMA_LOG_LEVEL_FATAL;
		level <= RPMA_LOG_LEVEL_DEBUG; level++) {
		/* setup time-related mock */
		TIME_MOCKS_INIT(config);
		/* setup frprintf mock */
		char expected_fprintf_output[256] = "";
		strcat(expected_fprintf_output, config->clock_gettime_error ||
		config->localtime_error || config->strftime_error ?
		TEST_TIME_ERROR_STR "*": TEST_TIME_STR "*");
		strcat(expected_fprintf_output, level2string(level));
		strcat(expected_fprintf_output, "*: " TEST_MESSAGE);
		expect_string(__wrap_fprintf, fprintf_output,
				expected_fprintf_output);
		rpma_log(level, NULL, TEST_LINE_NO, TEST_FUNCTION_NAME, "%s",
			TEST_MESSAGE);
	}
}

/*
 * init_fini__lifecycle -- log life-cycle scenario
 */
static void
init_fini__lifecycle(void **unused)
{
	/*
	 * log is opened in setup as it is done during loading of the library
	 *
	 * verify that logging to syslog works as expected
	 */
	log__to_stderr(unused);

	/* log shall not be reinitialized without closing it first */
	assert_int_equal(-1, rpma_log_init(NULL));

	/* verify that logging to syslog still works as expected */
	log__to_stderr(unused);

	/* close log */
	expect_function_call(closelog);
	rpma_log_fini();

	/* verify that no output is produced to syslog */
	rpma_log(RPMA_LOG_LEVEL_FATAL, TEST_FILE_NAME, TEST_LINE_NO,
		TEST_FUNCTION_NAME, "%s", TEST_MESSAGE);
	rpma_log(RPMA_LOG_LEVEL_FATAL, NULL, 0, NULL, "%s", TEST_MESSAGE);
};

/*
 * threshold related setup
 */
static struct threshold_config th_config = {
	rpma_log_stderr_set_threshold,
	rpma_log_stderr_get_threshold,
	RPMA_LOG_LEVEL_DEBUG, RPMA_LOG_DISABLED
};

/*
 * different time mocks configurations
 */
static mock_config config_no_error = {
	0, 0, 0, &th_config
};

static mock_config config_gettime_error = {
	1, 0, 0, &th_config
};

static mock_config config_localtime_error = {
	0, 1, 0, &th_config
};

static mock_config config_strftime_error = {
	0, 0, 1, &th_config
};

const struct CMUnitTest log_test_to_stderr[] = {

	/* threshold setters/getters tests */
	cmocka_unit_test_prestate_setup_teardown(set_threshold__invalid,
		setup_threshold, NULL, &th_config),
	cmocka_unit_test_prestate_setup_teardown(set_threshold__all,
		setup_threshold, NULL, &th_config),

	/* logging with levels out of threshold */
	cmocka_unit_test_prestate_setup_teardown(
		log__out_of_threshold,
		setup_log, teardown_log, &th_config),

	/*
	 * logging to stderr with file information
	 * different time related mocks behaviors
	 */
	cmocka_unit_test_prestate_setup_teardown(
		log__to_stderr,
		setup_log_stderr, teardown_log_stderr, &config_no_error),
	cmocka_unit_test_prestate_setup_teardown(
		log__to_stderr,
		setup_log_stderr, teardown_log_stderr, &config_gettime_error),
	cmocka_unit_test_prestate_setup_teardown(
		log__to_stderr,
		setup_log_stderr, teardown_log_stderr, &config_localtime_error),
	cmocka_unit_test_prestate_setup_teardown(
		log__to_stderr,
		setup_log_stderr, teardown_log_stderr, &config_strftime_error),



	/*
	 * Logging to stderr without file and function information
	 * different time related mocks behaviors
	 */
	cmocka_unit_test_prestate_setup_teardown(
		log__to_stderr_file_name_function_name_NULL,
		setup_log_stderr, teardown_log_stderr, &config_no_error),
	cmocka_unit_test_prestate_setup_teardown(
			log__to_stderr_file_name_function_name_NULL,
		setup_log_stderr, teardown_log_stderr, &config_gettime_error),
	cmocka_unit_test_prestate_setup_teardown(
		log__to_stderr_file_name_function_name_NULL,
		setup_log_stderr, teardown_log_stderr, &config_localtime_error),
	cmocka_unit_test_prestate_setup_teardown(
		log__to_stderr_file_name_function_name_NULL,
		setup_log_stderr, teardown_log_stderr, &config_strftime_error),

	/*
	 * Logging to stderr without file information
	 * different time related mocks behaviors
	 */
	cmocka_unit_test_prestate_setup_teardown(
		log__to_stderr_file_name_NULL,
		setup_log_stderr, teardown_log_stderr, &config_no_error),
	cmocka_unit_test_prestate_setup_teardown(
		log__to_stderr_file_name_NULL,
		setup_log_stderr, teardown_log_stderr, &config_gettime_error),
	cmocka_unit_test_prestate_setup_teardown(
		log__to_stderr_file_name_NULL,
		setup_log_stderr, teardown_log_stderr, &config_localtime_error),
	cmocka_unit_test_prestate_setup_teardown(
		log__to_stderr_file_name_NULL,
		setup_log_stderr, teardown_log_stderr, &config_strftime_error),


	/* logging to syslog life cycle */
	cmocka_unit_test_prestate_setup_teardown(
		init_fini__lifecycle,
		setup_log_stderr,
		teardown_log_stderr,
		&config_no_error),
	cmocka_unit_test(NULL)

};
