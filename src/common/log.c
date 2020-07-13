// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020, Intel Corporation */

/*
 * log.c -- support for logging output to either syslog or stderr or
 *	via user defined function
 */

#include <stddef.h>
#include <stdarg.h>
#include <syslog.h>
#include <time.h>
#include <string.h>
#include <ctype.h>
#include "librpma.h"
#include "log_internal.h"

static const char *const rpma_level_names[] = {
	[RPMA_LOG_LEVEL_FATAL]	= "FATAL",
	[RPMA_LOG_LEVEL_ERROR]	= "ERROR",
	[RPMA_LOG_LEVEL_WARNING]	= "WARNING",
	[RPMA_LOG_LEVEL_NOTICE]	= "NOTICE",
	[RPMA_LOG_LEVEL_INFO]	= "INFO",
	[RPMA_LOG_LEVEL_DEBUG]	= "DEBUG",
};

/*
 * Log function - pointer to the main logging function.
 * It is set by default to default_log_function() but could be set to custom
 * logging function in rpma_log_init().
 *
 * Logging does not work if this pointer is set to NULL.
 */
static log_function *Log_function;

/*
 * rpma_log_init_default -- enable logging to syslog (and stderr)
 * during library load
 */
#ifndef RPMA_LOG_INIT_DEFAULT_OFF
__attribute__((constructor))
static void
rpma_log_init_default(void)
{
	rpma_log_init(NULL);
#ifdef DEBUG
	rpma_log_set_level(RPMA_LOG_LEVEL_DEBUG);
	rpma_log_stderr_set_level(RPMA_LOG_LEVEL_WARNING);
#else
	rpma_log_set_level(RPMA_LOG_LEVEL_WARNING);
	rpma_log_stderr_set_level(RPMA_LOG_DISABLED);
#endif
}
/*
 * rpma_log_fini_default -- disable logging during library unload
 */
__attribute__((destructor))
static void
rpma_log_fini_default(void)
{
	rpma_log_fini();
}
#endif

/*
 * logging level thresholds
 */
/* logging to syslog level threshold */
enum rpma_log_level Rpma_log_syslog_level = RPMA_LOG_DISABLED;
/* logging on stderr level threshold */
enum rpma_log_level Rpma_log_stderr_level = RPMA_LOG_DISABLED;

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
	if (0 > snprintf(buf, buf_size, "[%s.%06ld] ", date, usec)) {
		*buf = '\0';
		return;
	}
}

/*
 * default_log_function -- default logging function used to log a message
 * to syslog and/or stderr
 *
 * Message is started with prefix composed from file, line, func parameters
 * followed by string pointed by format. If format includes format specifiers
 * (subsequences beginning with %), the additional arguments following format
 * are formatted and inserted in the message.
 *
 * ASSUMPTIONS:
 * - level >= RPMA_LOG_LEVEL_FATAL && level <= RPMA_LOG_LEVEL_DEBUG
 * - file == NULL || (file != NULL && function != NULL)
 */
static void
default_log_function(int level, const char *file, const int line,
		const char *func, const char *format, va_list arg)
{
	int severity = LOG_INFO;
	char prefix[256] = "";
	char timestamp[45] = "";
	char message[1024] = "";

	if (level > Rpma_log_stderr_level && level > Rpma_log_syslog_level)
		return;

	switch (level) {
	case RPMA_LOG_LEVEL_FATAL:
		severity = LOG_CRIT;
		break;
	case RPMA_LOG_LEVEL_ERROR:
		severity = LOG_ERR;
		break;
	case RPMA_LOG_LEVEL_WARNING:
		severity = LOG_WARNING;
		break;
	case RPMA_LOG_LEVEL_NOTICE:
		severity = LOG_NOTICE;
		break;
	case RPMA_LOG_LEVEL_INFO:
		severity = LOG_INFO;
		break;
	case RPMA_LOG_LEVEL_DEBUG:
		severity = LOG_DEBUG;
		break;
	case RPMA_LOG_DISABLED:
	default:
		return;
	}

	if (0 > vsnprintf(message, sizeof(message), format, arg))
		return;

	if (file)
		snprintf(prefix, sizeof(prefix), "%s:%4d:%s: *%s*: ",
			file, line, func, rpma_level_names[level]);
	else
		prefix[0] = '\0';

	if (level <= Rpma_log_stderr_level) {
		get_timestamp_prefix(timestamp, sizeof(timestamp));
		fprintf(stderr, "%s%s%s", timestamp, prefix, message);
	}

	if (level <= Rpma_log_syslog_level)
		syslog(severity, "%s%s", prefix, message);
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
		(default_log_function != custom_log_function)) {
		Log_function = custom_log_function;
	} else {
		Log_function = default_log_function;
		openlog("rpma", LOG_PID, LOG_LOCAL7);
#ifdef DEBUG
		rpma_log_set_level(RPMA_LOG_LEVEL_DEBUG);
		rpma_log_stderr_set_level(RPMA_LOG_LEVEL_WARNING);
#else
		rpma_log_set_level(RPMA_LOG_LEVEL_NOTICE);
		rpma_log_stderr_set_level(RPMA_LOG_LEVEL_ERROR);
#endif
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
	if (default_log_function == Log_function)
		closelog();
	Log_function = NULL;
}

/*
 * rpma_log -- convert additional format arguments to variable argument list
 * and call main logging function rpma_vlog to write messages either
 * to the syslog and to stderr or call custom log function provided
 * via rpma_log_init.
 */
void
rpma_log(enum rpma_log_level level, const char *file, const int line,
	const char *func, const char *format, ...)
{
	va_list arg;
	va_start(arg, format);
	rpma_vlog(level, file, line, func, format, arg);
	va_end(arg);
}

/*
 * rpma_vlog -- call Log_function to write messages either
 * to the syslog and to stderr via default_log_function() function
 * or call the custom log function provided via rpma_log_init.
 */
void
rpma_vlog(enum rpma_log_level level, const char *file, const int line,
	const char *func, const char *format, va_list arg)
{
	if ((NULL != file && NULL == func) || (NULL == format))
		return;

	if (Log_function)
		Log_function(level, file, line, func, format, arg);
}

/*
 * rpma_log_set_level -- set the log level threshold for syslog messages.
 */
int
rpma_log_set_level(enum rpma_log_level level)
{
	if (level < RPMA_LOG_DISABLED || level > RPMA_LOG_LEVEL_DEBUG)
		return RPMA_E_INVAL;

	Rpma_log_syslog_level = level;
	return 0;
}

/*
 * rpma_log_get_level -- get the current log level threshold for syslog
 * messages.
 */
enum rpma_log_level
rpma_log_get_level(void)
{
	return Rpma_log_syslog_level;
}

/*
 * rpma_log_stderr_set_level -- set the current log level threshold
 * for printing to stderr.
 */
int
rpma_log_stderr_set_level(enum rpma_log_level level)
{
	if (level < RPMA_LOG_DISABLED || level > RPMA_LOG_LEVEL_DEBUG)
		return RPMA_E_INVAL;

	Rpma_log_stderr_level = level;
	return 0;
}

/*
 * rpma_log_stderr_get_level -- get the current log level threshold
 * for printing to stderr.
 */
enum rpma_log_level
rpma_log_stderr_get_level(void)
{
	return Rpma_log_stderr_level;
}
