/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright 2020, Intel Corporation */

/*
 * librpma_log.h -- public API fpr librpma logging interfaces
 */

#ifndef LIBRPMA_LOG_H
#define LIBRPMA_LOG_H
#include <stdio.h>
#include <librpma.h> // for RDMA_E_INVAL

// XXX documentation to be updated to txt2man
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
void rpma_log_init(logfunc *logf);

/*
 * Close the currently active log. Messages after this call
 * will be dropped.
 */
void rpma_log_fini(void);

enum rpma_log_level {
	RPMA_LOG_DISABLED = -1, // all messages will be suppressed
	RPMA_LOG_ERROR, // error that cause library to stop working properly
	RPMA_LOG_WARN, // errors that could be handled in the upper level
	RPMA_LOG_NOTICE, // non-massive info e.g. connection established
	RPMA_LOG_INFO, // massive info e.g. every write operation indication
	RPMA_LOG_DEBUG, // debug info e.g. write operation dump
};

/*
 * Set the log level threshold to log messages. Messages with a higher
 * level than this are ignored.
 *
 * \param level Log level threshold to set to log messages.
 *
 * \return RDMA_E_INVAL if level out of scope otherwise 0
 *
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
 *
 * \return RDMA_E_INVAL if level out of scope otherwise 0
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
 *
 * \return RDMA_E_INVAL if level out of scope otherwise 0
 *
 */
int rpma_log_set_print_level(enum rpma_log_level level);

/*
 * Get the current log level print threshold.
 *
 * \return the current log level print threshold.
 */
enum rpma_log_level rpma_log_get_print_level(void);

#endif /* LIBRPMA_LOG_H */
