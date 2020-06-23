// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020, Intel Corporation */

/*
 * \file
 * Logging interfaces
 */

#ifndef LIBRPMA_LOG_H
#define LIBRPMA_LOG_H
#include <stdio.h>
#ifdef __cplusplus
extern "C" {
#endif

/*
 * for passing user-provided log call
 *
 * \param level Log level threshold.
 * \param file Name of the current source file.
 * \param line Current source file line.
 * \param func Current source function name.
 * \param format Format string to the message.
 * \param args Additional arguments for format string.
 */
typedef void logfunc(int level, const char *file, const int line,
		const char *func, const char *format, va_list args);

/*
 * Initialize the logging module. Messages prior
 * to this call will be dropped.
 */
void rpma_log_open(logfunc *logf);

/*
 * Close the currently active log. Messages after this call
 * will be dropped.
 */
void rpma_log_close(void);

enum rpma_log_level {
	/* All messages will be suppressed. */
	RPMA_LOG_DISABLED = -1,
	RPMA_LOG_ERROR,
	RPMA_LOG_WARN,
	RPMA_LOG_NOTICE,
	RPMA_LOG_INFO,
	RPMA_LOG_DEBUG,
};

/*
 * Set the log level threshold to log messages. Messages with a higher
 * level than this are ignored.
 *
 * \param level Log level threshold to set to log messages.
 */
int rpma_log_set_level(enum rpma_log_level level);

/*
 * Get the current log level threshold.
 *
 * \return the current log level threshold.
 */
enum rpma_log_level rpma_log_get_level(void);

/*
 * Set the log level threshold to include stack trace in log messages.
 * Messages with a higher level than this will not contain stack trace. You
 * can use \c RPMA_LOG_DISABLED to completely disable stack trace printing
 * even if it is supported.
 *
 * \note This function has no effect if LIBRPMA is built without stack trace
 *  printing support.
 *
 * \param level Log level threshold for stacktrace.
 */
int rpma_log_set_backtrace_level(enum rpma_log_level level);

/*
 * Get the current log level threshold for showing stack trace in log message.
 *
 * \return the current log level threshold for stack trace.
 */
enum rpma_log_level rpma_log_get_backtrace_level(void);

/*
 * Set the current log level threshold for printing to stderr.
 * Messages with a level less than or equal to this level
 * are also printed to stderr. You can use \c RPMA_LOG_DISABLED to completely
 * suppress log printing.
 *
 * \param level Log level threshold for printing to stderr.
 */
int rpma_log_set_print_level(enum rpma_log_level level);

/*
 * Get the current log level print threshold.
 *
 * \return the current log level print threshold.
 */
enum rpma_log_level rpma_log_get_print_level(void);


#ifdef __cplusplus
}
#endif

#endif /* LIBRPMA_LOG_H */
