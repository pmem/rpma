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


#define TEST_TIME_STR "[1970-01-01 00:00:00.000000] "
#define TEST_TIME_ERROR_STR "[time error] "

/*
 * __wrap_clock_gettime() -- clock_gettime() mock
 */
int
__wrap_clock_gettime(clockid_t __clock_id, struct timespec *__tp)
{
	assert_int_equal(__clock_id, CLOCK_REALTIME);
	int err = mock_type(int);
	if (err)
		return err;

	__tp->tv_nsec = 0;
	__tp->tv_sec = 0;
	return 0;
}

/*
 * __wrap_localtime() -- localtime() mock
 */
extern struct tm *__real_localtime(const time_t *__timer);

struct tm *
__wrap_localtime(const time_t *__timer)
{
	assert_non_null(__timer);
	int ret = mock_type(int);
	if (!ret)
		return NULL;

	/*
	 * return fix predefine point in time
	 * alternative approach with real localtime() might add daylight
	 * shift causing indeterministic tests result
	 */
	static struct tm value = {0};
	value.tm_yday = 1; // XXX combine this with TEST_TIME_STR
	value.tm_mday = 1;
	value.tm_year = 70;
	return &value;
}

/*
 * __wrap_strftime() -- strftime mock
 */
extern size_t __real_strftime(char *__restrict __s, size_t __maxsize,
	const char *__restrict __format, const struct tm *__restrict __tp);

size_t
__wrap_strftime(char *__restrict __s, size_t __maxsize,
	const char *__restrict __format, const struct tm *__restrict __tp)
{
	int ret = mock_type(int);
	if (!ret)
		return 0;

	return __real_strftime(__s, __maxsize, __format, __tp);
}

typedef struct {
	const int clock_gettime_error;
	const int localtime_error;
	const int strftime_error;
	struct threshold_config *tresholds;
} mock_config;

/*
 * setup_log_stderr() -- wraper for setup_log_syslog() call
 */
int
setup_log_stderr(void **config_ptr)
{
	mock_config *config = (mock_config *)*config_ptr;
	struct threshold_config *th_config = config->tresholds;
	return setup_log((void **)&th_config);
}

/*
 * teardown_log_stderr() -- wraper for teardown_log_syslog() call
 */
int
teardown_log_stderr(void **config_ptr)
{
	mock_config *config = (mock_config *)*config_ptr;
	return teardown_log((void **)&(config->tresholds));
}

#define TIME_MOCKS_INIT(x) \
	if (x->clock_gettime_error) { \
		will_return(__wrap_clock_gettime, -1); \
	} else if (x->localtime_error) { \
		will_return(__wrap_clock_gettime, 0); \
		will_return(__wrap_localtime, 0); \
	} else if (x->strftime_error) { \
		will_return(__wrap_clock_gettime, 0); \
		will_return(__wrap_localtime, 1); \
		will_return(__wrap_strftime, 0); \
	} else { \
		will_return(__wrap_clock_gettime, 0); \
		will_return(__wrap_localtime, 1); \
		will_return(__wrap_strftime, 1); \
	}

/*
 * log__to_stderr -- successful logging to stderr with file related
 * information like file name, line number and function name
 */
static void
log__to_stderr(void **config_ptr)
{
	mock_config *config = (mock_config *)*config_ptr;
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

static struct threshold_config th_config = {
	rpma_log_stderr_set_threshold,
	rpma_log_stderr_get_threshold,
	RPMA_LOG_LEVEL_DEBUG, RPMA_LOG_DISABLED
};

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

	/* logging to stderr with file information */
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



	/* Logging to stderr without file and function information */
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

	/* Logging to stderr without file information */
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
