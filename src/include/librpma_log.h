/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright 2020, Intel Corporation */

/*
 * librpma_log.h -- definitions of librpma logging mechanism
 */

#ifndef LIBRPMA_LOG_H
#define LIBRPMA_LOG_H

#include <stdio.h>
#include <librpma.h> /* for RPMA_E_INVAL */

/*
 * Available log levels in librpma.
 * Log levels (except RPMA_LOG_DISABLED) are used in logging API calls to
 * indicate logging message severity.
 * Log levels are also set to define thresholds for loggin.
 */
typedef enum {
	/* all messages will be suppressed */
	RPMA_LOG_DISABLED = -1,
	/* an error that causes the library to stop working immediately */
	RPMA_LOG_LEVEL_FATAL,
	/* an error that causes the library to stop working properly */
	RPMA_LOG_LEVEL_ERROR,
	/* an errors that could be handled in the upper level */
	RPMA_LOG_LEVEL_WARNING,
	/* non-massive info e.g. connection established */
	RPMA_LOG_LEVEL_NOTICE,
	/* massive info e.g. every write operation indication */
	RPMA_LOG_LEVEL_INFO,
	/* debug info e.g. write operation dump */
	RPMA_LOG_LEVEL_DEBUG,
} rpma_log_level;

/*
 * for passing user-defined log call
 */
typedef void log_function(rpma_log_level, /* log level threshold */
	const char *file,	/* name of the current source file */
	const int line,		/* current source file line */
	const char *function,	/* current source function name */
	const char *format,	/* format string for the message */
	va_list args);		/* additional arguments for format string */

/** 3
 * rpma_log_init - initialize logging module of the librpma
 *
 * SYNOPSIS
 *
 * #include <librpma_log.h>
 *
 * typedef void log_function(
 *	rpma_log_level level,
 *	const char *file,
 *	const int line,
 *	const char *function,
 *	const char *format,
 *	va_list args);
 *
 * int rpma_log_init(log_function *user_defined_log_function);
 *
 * DESCRIPTION
 * rpma_log_init() initializes the logging module. Messages prior to this call
 * will be dropped. Logging messages are written either to syslog(3)/stderr(3)
 * or delivered to end-user application via function given by
 * user_defined_log_function parameter.
 * Logging thresholds to syslog(3)/stderr(3)
 * are set using rpma_log_syslog_set_threshold(3) and rpma_log_stderr_set_threshold(3).
 *
 * rpma_log_init() is automatically called when librpma library is loaded
 * and default thresholds are set:
 * - RPMA_LOG_WARNING for syslog(3)
 * - RPMA_LOG_DISABLED for stderr(3).
 *
 * Logging to syslog(3)/stderr(3) is disabled when user_defined_log_function
 * is provided. In such a case all messages are passed directly
 * to the given function.
 *
 * Parameters of a user-define log function are as follow:
 * - level - actual logging level of message - see rpma_log_level
 * - file - source file name where log message is produced. Could be set to NULL
 * and in such a case neither line nor function value are valid
 * - line - source file line number where log message is produced
 * - function - function name where log message is produced
 * - format - printf(3) like format of message
 * - args - va_list of arguments as described in \fIformat\fP
 *
 * ERRORS
 * rpma_log_init() can fail with the following error:
 *
 * - -1 - logging has already been started. Call rpma_log_fini (3) to close
 * currently active log.
 */
int rpma_log_init(log_function *user_defined_log_function);

/** 3
 * rpma_log_fini - close the currently active log
 *
 * SYNOPSIS
 *
 * #include <librpma_log.h>
 *
 * void
 * rpma_log_fini(void);
 *
 * DESCRIPTION
 * rpma_log_fini() close the currently active log. Messages after this call
 * will be dropped.
 */
void rpma_log_fini(void);


/** 3
 * rpma_log_syslog_set_threshold - set the log level threshold for syslog
 *
 * SYNOPSIS
 *
 * #include <librpma_log.h>
 *
 * int rpma_log_syslog_set_threshold(rpma_log_level level);
 *
 * typedef enum {
 *	RPMA_LOG_DISABLED,
 *	RPMA_LOG_LEVEL_ERROR,
 *	RPMA_LOG_LEVEL_WARNING,
 *	RPMA_LOG_LEVEL_NOTICE,
 *	RPMA_LOG_LEVEL_INFO,
 *	RPMA_LOG_LEVEL_DEBUG,
 * } rpma_log_level;
 *
 * DESCRIPTION
 * rpma_log_syslog_set_threshold()
 * set the log level threshold for log messages. Messages with a higher
 * level than this are ignored. RPMA_LOG_DISABLED
 *  shall be used to completely suppress writing to syslog(3).
 *
 * The threshold for stderr(3) is controlled separately via rpma_log_stderr_set_threshold()
 *
 * Available threshold levels are defined by rpma_log_level:
 * - RPMA_LOG_DISABLED - all messages will be suppressed
 * - RPMA_LOG_LEVEL_ERROR - error that cause library to stop working properly
 * - RPMA_LOG_LEVEL_WARNING - errors that could be handled in the upper level
 * - RPMA_LOG_LEVEL_NOTICE - non-massive info mainly related to public
 * API function completions e.g. connection established
 * - RPMA_LOG_LEVEL_INFO - massive info e.g. every write operation indication
 * - RPMA_LOG_LEVEL_DEBUG - debug info e.g. write operation dump
 *
 * RETURN VALUE
 * rpma_log_syslog_set_threshold() function returns 0 on success or error code on failure.
 *
 * ERRORS
 * rpma_log_syslog_set_threshold() can fail with the following error:
 *
 * - RPMA_E_INVAL - level out of scope
 *
 * NOTES
 * rpma_log_syslog_set_threshold() is automatically called during library load to set
 * default syslog(3) logging threshold to RPMA_LOG_LEVEL_WARNING
 *
 */

int rpma_log_syslog_set_threshold(rpma_log_level level);

/** 3
 * rpma_log_syslog_get_threshold - get the current log level threshold
 *
 * SYNOPSIS
 *
 * #include <librpma_log.h>
 *
 * rpma_log_level rpma_log_syslog_get_threshold(void);
 *
 * DESCRIPTION
 * rpma_log_syslog_get_threshold()
 * get the current log level threshold for messages written to syslog(3).
 *
 * RPMA_LOG_DISABLED indicates completely suppress writing to syslog(3).
 *
 * See rpma_log_syslog_set_threshold(3) for available thresholds.
 *
 * RETURN VALUE
 * rpma_log_syslog_get_threshold() returns actual log to syslog() threshold or
 * RPMA_LOG_DISABLED if logging to syslog() is disabled.
 *
 */
rpma_log_level rpma_log_syslog_get_threshold(void);

/** 3
 * rpma_log_stderr_set_threshold - set the log level threshold for stderr
 *
 * SYNOPSIS
 *
 * #include <librpma_log.h>
 *
 * int rpma_log_stderr_set_threshold( rpma_log_level level);
 *
 * DESCRIPTION
 * rpma_log_stderr_set_threshold()
 * set the log level threshold for log messages for stderr(3).
 * Messages with a higher level than this are not shown on stderr(3).
 * RPMA_LOG_DISABLED shall be used to completely suppress writing to stderr(3).
 *
 * For available threshold see rpma_log_syslog_set_threshold(3).
 *
 * RETURN VALUE
 * rpma_log_stderr_set_threshold() function returns 0 on success or error code
 * on failure.
 *
 * ERRORS
 * rpma_log_stderr_set_threshold() can fail with the following error:
 *
 * - RPMA_E_INVAL - level out of scope

 * NOTES
 * rpma_log_stderr_set_threshold() is automatically called during library load
 * to set default stderr(3) logging threshold to RPMA_LOG_DISABLED
 *
 */
int rpma_log_stderr_set_threshold(rpma_log_level level);

/** 3
 * rpma_log_stderr_get_threshold - get the current log level to stderr threshold
 *
 * SYNOPSIS
 *
 * #include <librpma_log.h>
 *
 * rpma_log_level rpma_log_stderr_get_threshold(void);
 *
 * DESCRIPTION
 * rpma_log_stderr_get_threshold()
 * get the current log level print to stderr threshold.
 *
 * RPMA_LOG_DISABLED indicates completely suppress printing on stderr(3).
 *
 * For available thresholds see rpma_log_syslog_set_threshold(3).
 *
 * RETURN VALUE
 * rpma_log_stderr_get_threshold() returns actual log print to stderr(3) threshold
 * or RPMA_LOG_DISABLED if log printing to stderr is disabled.
 */
rpma_log_level rpma_log_stderr_get_threshold(void);

#endif /* LIBRPMA_LOG_H */
