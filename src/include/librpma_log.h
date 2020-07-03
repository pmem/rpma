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
 * // logging level - see enum rpma_log_leve
 * int level,
 * // source file name where log message is produced
 * const char *file,
 * // source file line number here log message is produced
 * const int line,
 * // function name where log message is produced
 * const char *function,
 * // like printf(3) format o message
 * const char *format,
 * // message arguments (as described in \fIformat\fP)
 * va_list args);
 * .PP
 * Argument \fIfile \fRis set to NULL if no file related information
 * is given. In such case \fIline \fRand \fIfunction \fR are undefined.
 * .PP
 * \fBrpma_log_init\fP(NULL) is automatically called when librpma library
 * is loaded and following thresholds are set:
 * \fB RPMA_LOG_WARN \fRfor \fBsyslog \fP(3) and \fBRPMA_LOG_DISABLED
 * \fRfor \fBstderr \fP(3).
 * .PP
 * The log.c musy be compiled with
 * .B -DRPMA_LOG_INIT_AT_STARTUP_SUSPENDED
 * defined to disable log initialization at startup.
 *
 * ERRORS
 * rpma_log_init() can fail with the following errors:
 *
 * - -1 - logging has already been started. Call
 * .BR rpma_log_fini (3)
 * to close currently active log.
 */
int
rpma_log_init(logfunc *user_defined_log_function);

/** 3
 * rpma_log_finit - close the currently active log
 * SYNOPSIS
 * #include <librpma_log.h>
 *
 * void
 * rpma_log_finit(void);
 *
 * DESCRIPTION
 * .BR rpma_log_fini\fP()
 * close the currently active log. Messages after this call
 * will be dropped.
 */
void
rpma_log_fini(void);

enum rpma_log_level {
	/* all messages will be suppressed */
	RPMA_LOG_DISABLED = -1,
	/* error that cause library to stop working immediately */
	RPMA_LOG_FATAL,
	/* error that cause library to stop working properly */
	RPMA_LOG_ERROR,
	/* errors that could be handled in the upper level */
	RPMA_LOG_WARN,
	/* non-massive info e.g. connection established */
	RPMA_LOG_NOTICE,
	/* massive info e.g. every write operation indication */
	RPMA_LOG_INFO,
	/* debug info e.g. write operation dump */
	RPMA_LOG_DEBUG,
};

/** 3
 * rpma_log_set_level - set the log level threshold for syslog
 *
 * SYNOPSIS
 * #include <librpma_log.h>
 *
 * int rpma_log_set_level(enum rpma_log_level level);
 *
 * DESCRIPTION
 *
 * rpma_log_set_level()
 * set the log level threshold to log messages. Messages with a higher
 * level than this are ignored.
 * .B RPMA_LOG_DISABLE
 *  shall be used to completely suppress writing to syslog(3).
 * .PP
 * Threshold for
 * \fBstderr\fR is controlled separately via rpma_log_stderr_set_level()
 * .PP
 * Available threshold levels are defined by enum rpma_log_level:
 * .nf
 * enum rpma_log_level {
 * .in +8
 *	// all messages will be suppressed
 *	RPMA_LOG_DISABLED = -1,
 *	// error that cause library to stop working properly
 *	RPMA_LOG_ERROR,
 *	// errors that could be handled in the upper level
 *	RPMA_LOG_WARN,
 *	// non-massive info e.g. connection established
 *	RPMA_LOG_NOTICE,
 *	// massive info e.g. every write operation indication
 *	RPMA_LOG_INFO,
 *	// debug info e.g. write operation dump
 *	RPMA_LOG_DEBUG
 * }
 *
 * RETURN VALUE
 * rpma_log_set_level() function returns 0 on success or
 * error code on failure.
 *
 * ERRORS
 * rpma_log_set_level() can fail with the following errors:
 *
 * - RPMA_E_INVAL - level out of scope
 *
 */
int
rpma_log_set_level(enum rpma_log_level level);

/** 3
 * rpma_log_get_level - get the current log level threshold
 *
 * SYNOPSIS
 * #include <librpma_log.h>
 *
 * .B enum rpma_log_level
 * rpma_log_get_level(void);
 *
 * DESCRIPTION
 *
 * rpma_log_get_level()
 * get the current log level threshold to messages written to syslog(3).
 * .PP
 * .B RPMA_LOG_DISABLE
 * indicates completely suppress writing to syslog(3).
 * .PP
 * For available threshold see rpma_log_set_level(3).
 *
 * RETURN VALUE
 * rpma_log_get_level() returns actual log to syslog() threshold or
 * RPMA_LOG_DISABLE if logging to syslog() is disabled.
 *
 */
enum rpma_log_level
rpma_log_get_level(void);

/** 3
 * rpma_log_stderrset_level - set the log level threshold for stderr
 *
 * SYNOPSIS
 * #include <librpma_log.h>
 *
 * int rpma_log_stderrset_level(enum rpma_log_level level);
 *
 * DESCRIPTION
 *
 * rpma_log_stderr_set_level()
 * set the log level threshold to log messages for \fBstderr\fR.
 * Messages with a higher level than this are not shown on \fBstderr\fR.
 * .B RPMA_LOG_DISABLE
 * shall be used to completely suppress writing to \fBstderr\fR.
 * .PP
 * For available threshold see rpma_log_set_level(3).
 *
 * RETURN VALUE
 * rpma_log_stderr_set_level() function returns 0 on success or
 * error code on failure.
 *
 * ERRORS
 * rpma_log_stderr_set_level() can fail with the following errors:
 *
 * - RPMA_E_INVAL - level out of scope
 *
 */
int rpma_log_stderr_set_level(enum rpma_log_level level);

/** 3
 * rpma_log_stderr_get_level - get the current log level print threshold
 *
 * SYNOPSIS
 * #include <librpma_log.h>
 *
 * .B enum rpma_log_level
 * rpma_log_stderr_get_level(void);
 *
 * DESCRIPTION
 *
 * rpma_log_stderr_get_level()
 * get the current log level print to \fBstderr\fR threshold.
 * .PP
 * .B RPMA_LOG_DISABLE
 * indicates completely suppress printing on \fBstderr\fR.
 * .PP
 * For available threshold see rpma_log_set_level(3).
 *
 * RETURN VALUE
 * rpma_log_stderr_get_level() returns actual log print to \fBstderr\fR
 * threshold or
 * .B RPMA_LOG_DISABLE
 * if log printing to \fBstderr\fR is disabled.
 *
 */
enum rpma_log_level rpma_log_stderr_get_level(void);

#endif /* LIBRPMA_LOG_H */
