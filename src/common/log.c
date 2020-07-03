// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020, Intel Corporation */

/*
 * log.c -- support for logging output to either syslog or stderr or
 *          via user defined function
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
	[RPMA_LOG_FATAL]	= "FATAL",
	[RPMA_LOG_ERROR]	= "ERROR",
	[RPMA_LOG_WARN]		= "WARNING",
	[RPMA_LOG_NOTICE]	= "NOTICE",
	[RPMA_LOG_INFO]		= "INFO",
	[RPMA_LOG_DEBUG]	= "DEBUG",
};

/*
 * log function - used if not NULL (see rpma_log_open)
 * all logs end up there regardless of logging threshold
 */
static logfunc *log_function = NULL;


/*
 * to enable logging to syslog (and stderr) at startup
 */
#ifndef RPMA_LOG_INIT_AT_STARTUP_SUSPENDED
__attribute__((constructor)) static void rpma_log_init_default(void)
{
	rpma_log_init(NULL);
#ifdef DEBUG
	rpma_log_set_level(RPMA_LOG_DEBUG);
	rpma_log_stderr_set_level(RPMA_LOG_WARN);
#else
	rpma_log_set_level(RPMA_LOG_WARN);
	rpma_log_stderr_set_level(RPMA_LOG_DISABLED);
#endif
}

__attribute__((destructor)) static void rpma_log_fini_default(void)
{
	rpma_log_fini();
}
#endif

/*
 * loging level tresholds
 */
// syslog treshold
enum rpma_log_level Rpma_log_level = RPMA_LOG_DISABLED;
// stderr treshold
enum rpma_log_level Rpma_log_print_level = RPMA_LOG_DISABLED;

/*
 * Provide actual time in a readable string
 */
static void
get_timestamp_prefix(char *buf, size_t buf_size)
{
	struct tm *info;
	char date[24];
	struct timespec ts;
	long usec;

	clock_gettime(CLOCK_REALTIME, &ts);
	info = localtime(&ts.tv_sec);
	usec = ts.tv_nsec / 1000;

	strftime(date, sizeof(date), "%Y-%m-%d %H:%M:%S", info);
	snprintf(buf, buf_size, "[%s.%06ld] ", date, usec);
}

/*
 * default logging function used to log to syslog and/or stderr
 */
static void
default_log_function(int level, const char *file, const int line,
		const char *func, const char *format, va_list arg)
{
	int severity = LOG_INFO;
	char prefix[256] = "";
	char timestamp[45] = "";
	char message[1024] = "";

	if (level > Rpma_log_print_level && level > Rpma_log_level) {
		return;
	}

	switch (level) {
	case RPMA_LOG_FATAL:
		severity = LOG_CRIT;
		break;
	case RPMA_LOG_ERROR:
		severity = LOG_ERR;
		break;
	case RPMA_LOG_WARN:
		severity = LOG_WARNING;
		break;
	case RPMA_LOG_NOTICE:
		severity = LOG_NOTICE;
		break;
	case RPMA_LOG_INFO:
		severity = LOG_INFO;
		break;
	case RPMA_LOG_DEBUG:
		severity = LOG_DEBUG;
		break;
	case RPMA_LOG_DISABLED:
	default:
		return;
	}

	vsnprintf(message, sizeof(message), format, arg);

	if (file) {
		snprintf(prefix, sizeof(prefix), "%s:%4d:%s: *%s*: ", \
				file, line, func, rpma_level_names[level]);
	} else {
		prefix[0] = '\0';
	}
	if (level <= Rpma_log_print_level) {
		get_timestamp_prefix(timestamp, sizeof(timestamp));
		fprintf(stderr, "%s%s%s", timestamp, prefix, message);
	}
	if (level <= Rpma_log_level) {
		syslog(severity, "%s%s", prefix, message);
	}

}

/* public librpma log API */

/*
 * Initialize the logging module. Messages prior
 * to this call will be dropped.
 */
int
rpma_log_init(logfunc *custom_log_function)
{
	if (NULL != log_function) {
		return -1; /* log has already been opened */
	}
	if (custom_log_function &&
		(default_log_function != custom_log_function)) {
		log_function = custom_log_function;
	} else {
		log_function = default_log_function;
		openlog("rpma", LOG_PID, LOG_LOCAL7);
#ifdef DEBUG
		rpma_log_set_level(RPMA_LOG_DEBUG);
		rpma_log_stderr_set_level(RPMA_LOG_WARN);
#else
		rpma_log_set_level(RPMA_LOG_NOTICE);
		rpma_log_stderr_set_level(RPMA_LOG_ERROR);
#endif
	}
	return 0;
}

/*
 * Close the currently active log. Messages after this call
 * will be dropped.
 */
void
rpma_log_fini(void)
{
	if (default_log_function == log_function) {
		closelog();
	}
	log_function = NULL;
}

/*
 * Write messages either to the syslog and to stderr
 * or call user defined log function.
 */
void
rpma_log(enum rpma_log_level level, const char *file, const int line,
		const char *func, const char *format, ...)
{
	va_list arg;
	if (NULL != file && NULL == func) {
		return;
	}
	if (NULL == format) {
		return;
	}
	va_start(arg, format);
	rpma_vlog(level, file, line, func, format, arg);
	va_end(arg);
}

/*
 * Write messages either to the syslog and to stderr
 * or call user defined log function.
 */
void
rpma_vlog(enum rpma_log_level level, const char *file, const int line,
		const char *func, const char *format, va_list arg)
{
	if (log_function) {
		log_function(level, file, line, func, format, arg);
	}
}

/*
 * Set the log level threshold to log messages.
 */
int
rpma_log_set_level(enum rpma_log_level level)
{
	if (level < RPMA_LOG_DISABLED || level > RPMA_LOG_DEBUG) {
		return RPMA_E_INVAL;
	}
	Rpma_log_level = level;
	return 0;
}

/*
 * Get the current log level threshold.
 */
enum rpma_log_level
rpma_log_get_level(void)
{
	return Rpma_log_level;
}

/*
 * Set the current log level threshold for printing to stderr.
 */
int
rpma_log_stderr_set_level(enum rpma_log_level level)
{
	if (level < RPMA_LOG_DISABLED || level > RPMA_LOG_DEBUG) {
		return RPMA_E_INVAL;
	}
	Rpma_log_print_level = level;
	return 0;
}

/*
 * Get the current log level threshold for printing to stderr.
 */
enum rpma_log_level
rpma_log_stderr_get_level(void)
{
	return Rpma_log_print_level;
}
