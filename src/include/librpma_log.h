/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright 2020, Intel Corporation */

/*
 * librpma_log.h -- public API to control librpma logging mechanism behavior
 */

#ifndef LIBRPMA_LOG_H
#define LIBRPMA_LOG_H
#include <stdio.h>
#include <librpma.h> /* for RDMA_E_INVAL */

/*
 * for passing user-defined log call
 */
typedef void logfunc(int level, /* log level threshold */
	const char *file,	/* name of the current source file */
	const int line,		/* current source file line */
	const char *function,	/* current source function name */
	const char *format,	/* format string to the message */
	va_list args);		/* additional arguments for format string */

/** 3
 * rpma_log_init - initialize logging module of the librpma
 * SYNOPSIS
 * #include <librpma_log.h>
 *
 * int rpma_log_init(logfunc *user_defined_log_function);
 *
 * DESCRIPTION
 * .BR rpma_log_init\fP()
 * initializes the logging module. Messages prior to this call
 * will be dropped. Logging messages are written either to
 * .BR syslog(3)
 * /
 * .BR stderr(3)
 * or delivered to end-user application via function given by
 * \fIuser_defined_log_function\fP
 * parameter.
 * .PP
 * Logging to
 * .BR syslog(3)
 * /
 * .BR stderr(3)
 * is enabled when no
 * .I user_defined_log_function
 * is provided.
 * Logging thresholds to
 * .BR syslog (3)
 * /
 * .BR stderr (3)
 * are set using
 * .BR rpma_log_set_level (3)
 * and
 * .BR rpma_log_stderr_set_level (3)
 * .PP
 * User could provide own function which will be called instead of writing to
 * .BR syslog (3)
 * /
 * .BR stderr (3)\fR.
 * .PP
 * No threshold are used in such case - all messages are passed to
 * the given function.
 * .PP
 * User definie function shall have following signature:
 * .nf
 * typedef void logfunc(
 * .in +8
 * int level,		// logging level - see enum rpma_log_leve
 * const char *file,	// source file name where log message is produced
 * const int line,	// source file line number here log message is produced
 * const char *function,	// function name where log message is produced
 * const char *format,		// like \fBprintf \fP() format o message
 * va_list args);	// message arguments (as described in \fIformat\fP)
 * .PP
 * Argument \fIfile \fRis set to NULL if no file related information
 * is given. In such case \fIline \fRand \fIfunction \fR are undefined.
 * .PP
 * \fBrpma_log_init\fP(NULL) is automatically called when librpma library
 * is loaded and following threshold are set:
 * \fB RPMA_LOG_WARN \fRfor \fBsyslog \fP(3) and \fBRPMA_LOG_DISABLED
 * \fRfor \fBstderr \fP(3). The log.c musy be compiled with
 * .\fB-DRPMA_LOG_INIT_AT_STARTUP_SUSPENDED \fRdefined to disable log
 * initialization at startup.
 * ERRORS
 * rpma_log_init() can fail with the following errors:
 *
 * - -1 - logging has already been started. Call
 * .BR rpma_log_fini (3)
 * to close currently active log.
 */
int rpma_log_init(logfunc *user_defined_log_function);

/*
 * Close the currently active log. Messages after this call
 * will be dropped.
 */
/** 3
 * rpma_log_finit - close the currently active log
 * SYNOPSIS
 * #include <librpma_log.h>
 *
 * void rpma_log_finit(void);
 *
 * DESCRIPTION
 * .BR rpma_log_fini\fP()
 *  close the currently active log. Messages after this call
 * will be dropped.
 */
void rpma_log_fini(void);

enum rpma_log_level {
	RPMA_LOG_DISABLED = -1, /* all messages will be suppressed */
	RPMA_LOG_ERROR, /* error that cause library to stop working properly */
	RPMA_LOG_WARN, /* errors that could be handled in the upper level */
	RPMA_LOG_NOTICE, /* non-massive info e.g. connection established */
	RPMA_LOG_INFO, /* massive info e.g. every write operation indication */
	RPMA_LOG_DEBUG, /* debug info e.g. write operation dump */
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
 * can use RPMA_LOG_DISABLED to completely disable stack trace printing
 * even if it is supported.
 *
 * \note This function has no effect if librpma is built without stack trace
 * printing support.
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
 * are also printed to stderr. You can use RPMA_LOG_DISABLED to completely
 * suppress log printing.
 *
 * \param level Log level threshold for printing to stderr.
 *
 * \return RDMA_E_INVAL if level out of scope otherwise 0
 *
 */
int rpma_log_stderr_set_level(enum rpma_log_level level);

/*
 * Get the current log level print threshold.
 *
 * \return the current log level print threshold.
 */
enum rpma_log_level rpma_log_stderr_get_level(void);

#endif /* LIBRPMA_LOG_H */
