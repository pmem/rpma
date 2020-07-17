// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020, Intel Corporation */

/*
 * log.c -- support for logging output to either syslog or stderr or
 * via user defined function
 */

#include <stdarg.h>
#include <syslog.h>
#include <time.h>
#include <string.h>
#include "log_internal.h"

static const char *const rpma_log_level_names[] = {
	[RPMA_LOG_LEVEL_FATAL]	= "FATAL",
	[RPMA_LOG_LEVEL_ERROR]	= "ERROR",
	[RPMA_LOG_LEVEL_WARNING] = "WARNING",
	[RPMA_LOG_LEVEL_NOTICE]	= "NOTICE",
	[RPMA_LOG_LEVEL_INFO]	= "INFO",
	[RPMA_LOG_LEVEL_DEBUG]	= "DEBUG",
};

static const int rpma_log_level_syslog_severity[] = {
	[RPMA_LOG_LEVEL_FATAL]	= LOG_CRIT,
	[RPMA_LOG_LEVEL_ERROR]	= LOG_ERR,
	[RPMA_LOG_LEVEL_WARNING] = LOG_WARNING,
	[RPMA_LOG_LEVEL_NOTICE]	= LOG_NOTICE,
	[RPMA_LOG_LEVEL_INFO]	= LOG_INFO,
	[RPMA_LOG_LEVEL_DEBUG]	= LOG_DEBUG,
};
/*
 * Log function - pointer to the main logging function.
 * It is set by default to rpma_log_function() but could be set to custom
 * logging function in rpma_log_init().
 *
 * Logging is turned-off if this pointer is set to NULL.
 */
static log_function *Log_function;

/*
 * rpma_log_init_default -- enable logging to syslog (and stderr)
 * during loading of the library.
 */
__attribute__((constructor))
static void
rpma_log_init_default(void)
{
	rpma_log_init(NULL);
}

/*
 * rpma_log_fini_default -- disable logging during unloading the library.
 */
__attribute__((destructor))
static void
rpma_log_fini_default(void)
{
	rpma_log_fini();
}

/* threshold level for logging to syslog */
static rpma_log_level Rpma_log_syslog_threshold = RPMA_LOG_DISABLED;

/* threshold level for logging to stderr */
static rpma_log_level Rpma_log_stderr_threshold = RPMA_LOG_DISABLED;

/*
 * get_timestamp_prefix -- provide actual time in a readable string
 *
 * ASSUMPTIONS:
 * - buf != NULL
 */
static void
get_timestamp_prefix(char *buf, size_t buf_size)
{
	struct tm *info;
	char date[24];
	struct timespec ts;
	long usec;

	if (clock_gettime(CLOCK_REALTIME, &ts) ||
	    (NULL == (info = localtime(&ts.tv_sec)))) {
		snprintf(buf, buf_size, "[unknown time] ");
		return;
	}

	usec = ts.tv_nsec / 1000;
	if (!strftime(date, sizeof(date), "%Y-%m-%d %H:%M:%S", info)) {
		snprintf(buf, buf_size, "[unknown time] ");
		return;
	}

	if (snprintf(buf, buf_size, "[%s.%06ld] ", date, usec) < 0) {
		*buf = '\0';
		return;
	}
}

/*
 * rpma_level2syslog_severity - logging level to syslog severity conversion.
 *
 * ASSUMPTIONS:
 * - level != RPMA_LOG_DISABLE
 */
static inline int
rpma_log_level2syslog_severity(rpma_log_level level)
{
	return rpma_log_level_syslog_severity[level];
};

/*
 * rpma_log_function -- default logging function used to log a message
 * to syslog and/or stderr
 *
 * The message is started with prefix composed from file, line, func parameters
 * followed by string pointed by format. If format includes format specifiers
 * (subsequences beginning with %), the additional arguments following format
 * are formatted and inserted in the message.
 *
 * ASSUMPTIONS:
 * - level >= RPMA_LOG_LEVEL_FATAL && level <= RPMA_LOG_LEVEL_DEBUG
 * - file == NULL || (file != NULL && function != NULL)
 */
static void
rpma_log_function(rpma_log_level level, const char *file_name,
	const int line_no, const char *function_name,
	const char *message_format, va_list arg)
{
	char prefix[256] = "";
	char timestamp[45] = "";
	char message[1024] = "";

	if (level > Rpma_log_stderr_threshold &&
	    level > Rpma_log_syslog_threshold)
		return;

	if (vsnprintf(message, sizeof(message), message_format, arg) < 0)
		return;

	if (file_name) {
		if (snprintf(prefix, sizeof(prefix), "%s: %4d: %s: *%s*: ",
				file_name, line_no, function_name,
				rpma_log_level_names[level]) < 0)
			strcpy(prefix, "[error prefix]: ");
	} else {
		prefix[0] = '\0';
	}

	if (level <= Rpma_log_stderr_threshold) {
		get_timestamp_prefix(timestamp, sizeof(timestamp));
		(void) fprintf(stderr, "%s%s%s", timestamp, prefix, message);
	}

	if (level <= Rpma_log_syslog_threshold) {
		if (level != RPMA_LOG_DISABLED) {
			syslog(rpma_log_level2syslog_severity(level),
				"%s%s", prefix, message);
		}
	}
}

/*
 * rpma_log -- convert additional format arguments to variable argument list
 * and call main logging function pointed by Log_function.
 */
void
rpma_log(rpma_log_level level, const char *file_name, const int line_no,
	const char *function_name, const char *message_format, ...)
{
	if ((NULL != file_name && NULL == function_name) ||
	    (NULL == message_format)) {
		return;
	}

	if (NULL == Log_function)
		return;

	va_list arg;
	va_start(arg, message_format);
	Log_function(level, file_name, line_no, function_name, message_format,
			arg);
	va_end(arg);
}

/* public librpma log API */

/*
 * rpma_log_init -- initialize the logging module. Messages prior to this call
 * will be dropped.
 */
int
rpma_log_init(log_function *custom_log_function)
{
	if (NULL != Log_function)
		return -1; /* log has already been opened */

	if (custom_log_function &&
		(rpma_log_function != custom_log_function)) {
		Log_function = custom_log_function;
	} else {
		Log_function = rpma_log_function;
		openlog("rpma", LOG_PID, LOG_LOCAL7);
		rpma_log_syslog_set_threshold(RPMA_LOG_LEVEL_SYSLOG_DEFAULT);
		rpma_log_stderr_set_threshold(RPMA_LOG_LEVEL_STDERR_DEFAULT);
	}

	return 0;
}


/*
 * rpma_log_fini -- close the currently active log. Messages after this call
 * will be dropped.
 */
void
rpma_log_fini(void)
{
	if (rpma_log_function == Log_function) {
		closelog();
	}
	Log_function = NULL;
}

/*
 * rpma_log_syslog_set_threshold -- set the log level threshold for
 * syslog's messages.
 */
int
rpma_log_syslog_set_threshold(rpma_log_level level)
{
	if (level < RPMA_LOG_DISABLED || level > RPMA_LOG_LEVEL_DEBUG)
		return -1;

	Rpma_log_syslog_threshold = level;
	return 0;
}

/*
 * rpma_log_syslog_get_threshold -- get the current log level threshold for
 * syslog messages.
 */
rpma_log_level
rpma_log_syslog_get_threshold(void)
{
	return Rpma_log_syslog_threshold;
}

/*
 * rpma_log_stderr_set_threshold -- set the current log level threshold
 * for printing to stderr.
 */
int
rpma_log_stderr_set_threshold(rpma_log_level level)
{
	if (level < RPMA_LOG_DISABLED || level > RPMA_LOG_LEVEL_DEBUG)
		return -1;

	Rpma_log_stderr_threshold = level;
	return 0;
}

/*
 * rpma_log_stderr_get_threshold -- get the current log level threshold
 * for printing to stderr.
 */
rpma_log_level
rpma_log_stderr_get_threshold(void)
{
	return Rpma_log_stderr_threshold;
}
