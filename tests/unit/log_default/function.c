// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020-2022, Intel Corporation */

/*
 * function.c -- rpma_log_default_function() unit tests
 */

/*
 * These unit tests assume the following order of calls:
 * 1. In any order:
 * - vsnprintf - generating the message
 * - snprintf - generating the file info (if available)
 * 2. In any order:
 * - syslog
 * - writing to stderr (if level <= the secondary threshold level)
 *
 * Where 'writing to stderr' is:
 * 1. in the following order:
 *     1. clock_gettime
 *     2. localtime_r
 *     3. strftime
 *     4. snprintf
 * 2. fprintf (no matter if the time-related sequence will succeed)
 */

#include <string.h>
#include <syslog.h>
#include <time.h>

#include "cmocka_headers.h"
#include "log_default.h"
#include "log_internal.h"
#include "mocks-stdio.h"
#include "mocks-time.h"
#include "mocks-glibc.h"
#include "test-common.h"

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

#define MOCK_LOG_LEVEL		RPMA_LOG_LEVEL_WARNING
#define MOCK_FILE_NAME		"foo_bar.c"
#define MOCK_FILE_NAME_ABSOLUTE	"/path/to/foo_bar.c"
#define MOCK_LINE_NUMBER	199
#define MOCK_FUNCTION_NAME	"foo_bar()"
#define MOCK_MESSAGE		"Message"
#define MOCK_FILE_ERROR_STR	"[file info error]: "

/*
 * These two arrays should be the exact copy of the arrays you may find in
 * the log_default.c module. There is no another way of validating their
 * contents other than your cautiousness during editing these.
 */
static char const rpma_log_level_names[6][9] = {
	[RPMA_LOG_LEVEL_FATAL]	=  "*FATAL* ",
	[RPMA_LOG_LEVEL_ERROR]	=  "*ERROR* ",
	[RPMA_LOG_LEVEL_WARNING] = "*WARN*  ",
	[RPMA_LOG_LEVEL_NOTICE]	=  "*NOTE*  ",
	[RPMA_LOG_LEVEL_INFO]	=  "*INFO*  ",
	[RPMA_LOG_LEVEL_DEBUG]	=  "*DEBUG* ",
};

static const int rpma_log_level_syslog_severity[] = {
	[RPMA_LOG_LEVEL_FATAL]	= LOG_CRIT,
	[RPMA_LOG_LEVEL_ERROR]	= LOG_ERR,
	[RPMA_LOG_LEVEL_WARNING] = LOG_WARNING,
	[RPMA_LOG_LEVEL_NOTICE]	= LOG_NOTICE,
	[RPMA_LOG_LEVEL_INFO]	= LOG_INFO,
	[RPMA_LOG_LEVEL_DEBUG]	= LOG_DEBUG,
};

typedef struct {
	int clock_gettime_error;
	int localtime_r_error;
	int strftime_error;
	int snprintf_no_eol;

	enum rpma_log_level secondary;

	char *path;
} mock_config;

/*
 * setup_thresholds -- setup logging thresholds
 */
int
setup_thresholds(void **config_ptr)
{
	mock_config *cfg = (mock_config *)*config_ptr;

	/*
	 * The main log threshold should not affect logging function behaviour.
	 */
	Rpma_log_threshold[RPMA_LOG_THRESHOLD] = RPMA_LOG_DISABLED;
	Rpma_log_threshold[RPMA_LOG_THRESHOLD_AUX] = cfg->secondary;

	return 0;
}

/*
 * function__RPMA_LOG_DISABLED -- call rpma_log_default_function() with RPMA_LOG_DISABLED
 */
void
function__RPMA_LOG_DISABLED(void **unused)
{
	/* run test */
	rpma_log_default_function(RPMA_LOG_DISABLED, MOCK_FILE_NAME,
			MOCK_LINE_NUMBER, MOCK_FUNCTION_NAME, MOCK_MESSAGE);
}

/*
 * function__vsnprintf_fail -- vsnprintf() fails
 */
void
function__vsnprintf_fail(void **unused)
{
	/* configure mocks */
	will_return(__wrap_vsnprintf, MOCK_STDIO_ERROR);
	will_return_maybe(__wrap_snprintf, MOCK_OK);

	/* run test */
	rpma_log_default_function(MOCK_LOG_LEVEL, MOCK_FILE_NAME,
			MOCK_LINE_NUMBER, MOCK_FUNCTION_NAME, MOCK_MESSAGE);
}

/*
 * function__snprintf_fail -- snprintf() fails
 */
void
function__snprintf_fail(void **unused)
{
	/* configure mocks */
	will_return(__wrap_vsnprintf, MOCK_OK);
	will_return(__wrap_snprintf, MOCK_STDIO_ERROR);
	will_return(syslog, MOCK_VALIDATE);
	expect_value(syslog, priority,
			rpma_log_level_syslog_severity[MOCK_LOG_LEVEL]);

	/* construct the resulting syslog message */
	char msg[MOCK_BUFF_LEN] = "";
	strcat(msg, rpma_log_level_names[MOCK_LOG_LEVEL]);
	strcat(msg, MOCK_FILE_ERROR_STR MOCK_MESSAGE);
	expect_string(syslog, syslog_output, msg);

	/* run test */
	rpma_log_default_function(MOCK_LOG_LEVEL, MOCK_FILE_NAME,
			MOCK_LINE_NUMBER, MOCK_FUNCTION_NAME, MOCK_MESSAGE);
}

/*
 * function__syslog_no_path -- syslog() without a path
 */
void
function__syslog_no_path(void **unused)
{
	/* configure mocks */
	will_return(__wrap_vsnprintf, MOCK_OK);
	will_return(syslog, MOCK_VALIDATE);
	expect_value(syslog, priority,
			rpma_log_level_syslog_severity[MOCK_LOG_LEVEL]);

	/* construct the resulting syslog message */
	char msg[MOCK_BUFF_LEN] = "";
	strcat(msg, rpma_log_level_names[MOCK_LOG_LEVEL]);
	strcat(msg, MOCK_MESSAGE);
	expect_string(syslog, syslog_output, msg);

	/* run test */
	rpma_log_default_function(MOCK_LOG_LEVEL, NULL, 0, NULL, MOCK_MESSAGE);
}

/*
 * function__syslog -- syslog() using either an absolute or relative path
 */
void
function__syslog(void **config_ptr)
{
	mock_config *config = (mock_config *)*config_ptr;

	/* configure mocks */
	will_return(__wrap_vsnprintf, MOCK_OK);
	will_return(__wrap_snprintf, MOCK_OK);
	will_return(syslog, MOCK_VALIDATE);
	expect_value(syslog, priority,
			rpma_log_level_syslog_severity[MOCK_LOG_LEVEL]);

	/* construct the resulting syslog message */
	char msg[MOCK_BUFF_LEN] = "";
	strcat(msg, rpma_log_level_names[MOCK_LOG_LEVEL]);
	strcat(msg, MOCK_FILE_NAME ": " STR(MOCK_LINE_NUMBER) ": "
		MOCK_FUNCTION_NAME ": " MOCK_MESSAGE);
	expect_string(syslog, syslog_output, msg);

	/* run test */
	rpma_log_default_function(MOCK_LOG_LEVEL, config->path,
			MOCK_LINE_NUMBER, MOCK_FUNCTION_NAME, MOCK_MESSAGE);
}

#define MOCK_TIME_OF_DAY {00, 00, 00, 1, 0, 70, 0, 365, 0}
#define MOCK_TIME_OF_DAY_STR "Jan 01 00:00:00"
#define MOCK_TIME_STR MOCK_TIME_OF_DAY_STR ".000000 "
#define MOCK_TIME_ERROR_STR "[time error] "
#define MOCK_PID 123456
#define MOCK_PID_AS_STR "["STR(MOCK_PID)"] "

static struct timespec Timespec = {0};
static struct tm Tm = MOCK_TIME_OF_DAY;

/*
 * configure time.h mocks
 */
#define MOCK_GET_TIMESTAMP_CONFIGURE(x) \
	if ((x)->clock_gettime_error) { \
		will_return(__wrap_clock_gettime, NULL); \
	} else if ((x)->localtime_r_error) { \
		will_return(__wrap_clock_gettime, &Timespec); \
		will_return(__wrap_localtime_r, &Timespec); \
		will_return(__wrap_localtime_r, NULL); \
	} else if ((x)->strftime_error) { \
		will_return(__wrap_clock_gettime, &Timespec); \
		will_return(__wrap_localtime_r, &Timespec); \
		will_return(__wrap_localtime_r, &Tm); \
		will_return(__wrap_strftime, MOCK_STRFTIME_ERROR); \
	} else if ((x)->snprintf_no_eol) { \
		will_return(__wrap_clock_gettime, &Timespec); \
		will_return(__wrap_localtime_r, &Timespec); \
		will_return(__wrap_localtime_r, &Tm); \
		will_return(__wrap_strftime, MOCK_STRFTIME_SUCCESS); \
		will_return(__wrap_snprintf, MOCK_SNPRINTF_NO_EOL); \
	} else { \
		will_return(__wrap_clock_gettime, &Timespec); \
		will_return(__wrap_localtime_r, &Timespec); \
		will_return(__wrap_localtime_r, &Tm); \
		will_return(__wrap_strftime, MOCK_STRFTIME_SUCCESS); \
		will_return(__wrap_snprintf, MOCK_OK); \
	}

#define MOCK_TIME_STR_EXPECTED(x) \
	(((x)->clock_gettime_error || (x)->localtime_r_error || \
			(x)->strftime_error || (x)->snprintf_no_eol) ? \
			MOCK_TIME_ERROR_STR : MOCK_TIME_STR)

/*
 * function__stderr_path -- fprintf(stderr) with a provided path
 */
static void
function__stderr_path(void **config_ptr)
{
	mock_config *config = (mock_config *)*config_ptr;

	/* configure mocks */
	will_return(__wrap_vsnprintf, MOCK_OK);
	will_return(__wrap_snprintf, MOCK_OK);
	will_return(syslog, MOCK_PASSTHROUGH);
	MOCK_GET_TIMESTAMP_CONFIGURE(config);
	will_return(__wrap_syscall, MOCK_PID);

	/* construct the resulting fprintf message */
	char msg[MOCK_BUFF_LEN] = "";
	strcat(msg, MOCK_TIME_STR_EXPECTED(config));
	strcat(msg, MOCK_PID_AS_STR);
	strcat(msg, rpma_log_level_names[MOCK_LOG_LEVEL]);
	strcat(msg, MOCK_FILE_NAME ": " STR(MOCK_LINE_NUMBER) ": "
		MOCK_FUNCTION_NAME ": " MOCK_MESSAGE);
	will_return(__wrap_fprintf, MOCK_VALIDATE);
	expect_string(__wrap_fprintf, fprintf_output, msg);

	/* enable syscall()'s mock only for test execution */
	enabled__wrap_syscall = true;

	/* run test */
	rpma_log_default_function(MOCK_LOG_LEVEL, config->path,
			MOCK_LINE_NUMBER, MOCK_FUNCTION_NAME, "%s",
			MOCK_MESSAGE);

	/* disable syscall()'s mock after test execution */
	enabled__wrap_syscall = false;
}

/*
 * function__stderr_no_path -- fprintf(stderr) without a provided path
 */
static void
function__stderr_no_path(void **config_ptr)
{
	mock_config *config = (mock_config *)*config_ptr;

	for (enum rpma_log_level level = RPMA_LOG_LEVEL_FATAL;
		level <= RPMA_LOG_LEVEL_DEBUG; level++) {

		/* configure mocks */
		will_return(__wrap_vsnprintf, MOCK_OK);
		will_return(syslog, MOCK_PASSTHROUGH);
		MOCK_GET_TIMESTAMP_CONFIGURE(config);
		will_return(__wrap_syscall, MOCK_PID);

		/* construct the resulting fprintf message */
		char msg[MOCK_BUFF_LEN] = "";
		strcat(msg, MOCK_TIME_STR_EXPECTED(config));
		strcat(msg, MOCK_PID_AS_STR);
		strcat(msg, rpma_log_level_names[MOCK_LOG_LEVEL]);
		strcat(msg, MOCK_MESSAGE);
		will_return(__wrap_fprintf, MOCK_VALIDATE);
		expect_string(__wrap_fprintf, fprintf_output, msg);

		/* enable syscall()'s mock only for test execution */
		enabled__wrap_syscall = true;

		/* run test */
		rpma_log_default_function(MOCK_LOG_LEVEL, NULL, 0, NULL, "%s",
				MOCK_MESSAGE);

		/* disable syscall()'s mock after test execution */
		enabled__wrap_syscall = false;
	}
}

/*
 * function__stderr_no_path_ALWAYS -- fprintf(stderr) without a provided path
 * for RPMA_LOG_LEVEL_ALWAYS
 */
static void
function__stderr_no_path_ALWAYS(void **config_ptr)
{
	mock_config *config = (mock_config *)*config_ptr;

	for (enum rpma_log_level level = RPMA_LOG_LEVEL_FATAL;
		level <= RPMA_LOG_LEVEL_DEBUG; level++) {

		/* configure mocks */
		will_return(__wrap_vsnprintf, MOCK_OK);
		MOCK_GET_TIMESTAMP_CONFIGURE(config);
		will_return(__wrap_syscall, MOCK_PID);

		/* construct the resulting fprintf message */
		char msg[MOCK_BUFF_LEN] = "";
		strcat(msg, MOCK_TIME_STR_EXPECTED(config));
		strcat(msg, MOCK_PID_AS_STR);
		strcat(msg, rpma_log_level_names[RPMA_LOG_LEVEL_DEBUG]);
		strcat(msg, MOCK_MESSAGE);
		will_return(__wrap_fprintf, MOCK_VALIDATE);
		expect_string(__wrap_fprintf, fprintf_output, msg);

		/* enable syscall()'s mock only for test execution */
		enabled__wrap_syscall = true;

		/* run test */
		rpma_log_default_function(RPMA_LOG_LEVEL_ALWAYS, NULL, 0, NULL, "%s", MOCK_MESSAGE);

		/* disable syscall()'s mock after test execution */
		enabled__wrap_syscall = false;
	}
}

/*
 * test configurations
 */
static mock_config config_no_stderr = {
	0, 0, 0, 0, RPMA_LOG_DISABLED, MOCK_FILE_NAME
};

static mock_config config_no_stderr_path_absolute = {
	0, 0, 0, 0, RPMA_LOG_DISABLED, MOCK_FILE_NAME_ABSOLUTE
};

static mock_config config_no_error = {
	0, 0, 0, 0, RPMA_LOG_LEVEL_DEBUG, MOCK_FILE_NAME
};

static mock_config config_gettime_error = {
	1, 0, 0, 0, RPMA_LOG_LEVEL_DEBUG, MOCK_FILE_NAME
};

static mock_config config_localtime_r_error = {
	0, 1, 0, 0, RPMA_LOG_LEVEL_DEBUG, MOCK_FILE_NAME
};

static mock_config config_strftime_error = {
	0, 0, 1, 0, RPMA_LOG_LEVEL_DEBUG, MOCK_FILE_NAME
};

static mock_config config_snprintf_no_eol = {
	0, 0, 0, 1, RPMA_LOG_LEVEL_DEBUG, MOCK_FILE_NAME
};

int
main(int argc, char *argv[])
{
	const struct CMUnitTest tests[] = {
		/* syslog & stderr common tests */
		cmocka_unit_test_prestate_setup_teardown(
			function__RPMA_LOG_DISABLED,
			setup_thresholds, NULL, &config_no_stderr),
		cmocka_unit_test_prestate_setup_teardown(
			function__vsnprintf_fail,
			setup_thresholds, NULL, &config_no_stderr),
		cmocka_unit_test_prestate_setup_teardown(
			function__snprintf_fail,
			setup_thresholds, NULL, &config_no_stderr),

		/* syslog tests */
		cmocka_unit_test_prestate_setup_teardown(
			function__syslog_no_path,
			setup_thresholds, NULL, &config_no_stderr),
		{"function__syslog_path_relative",
			function__syslog, setup_thresholds, NULL,
			&config_no_stderr},
		{"function__syslog_path_absolute",
			function__syslog, setup_thresholds, NULL,
			&config_no_stderr_path_absolute},

		/* stderr tests - time-related fails */
		{"function__stderr_path_gettime_error",
			function__stderr_path,
			setup_thresholds, NULL, &config_gettime_error},
		{"function__stderr_path_localtime_r_error",
			function__stderr_path,
			setup_thresholds, NULL, &config_localtime_r_error},
		{"function__stderr_path_strftime_error",
			function__stderr_path,
			setup_thresholds, NULL, &config_strftime_error},
		{"function__stderr_path_snprintf_no_eol",
			function__stderr_path,
			setup_thresholds, NULL, &config_snprintf_no_eol},

		/* stderr tests - positive */
		cmocka_unit_test_prestate_setup_teardown(
			function__stderr_path,
			setup_thresholds, NULL, &config_no_error),
		cmocka_unit_test_prestate_setup_teardown(
			function__stderr_no_path,
			setup_thresholds, NULL, &config_no_error),
		cmocka_unit_test_prestate_setup_teardown(
			function__stderr_no_path_ALWAYS,
			setup_thresholds, NULL, &config_no_error),
	};

	return cmocka_run_group_tests(tests, NULL, NULL);
}
