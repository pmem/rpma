/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright 2020, Intel Corporation */

/*
 * log_internal.h -- internal logging interfaces used by the librpma.
 */

#ifndef LOG_INTERNAL_H
#define LOG_INTERNAL_H
#include "librpma_log.h"

#define RPMA_LOG_NOTICE(...) \
	rpma_log(RPMA_LOG_LEVEL_NOTICE, __FILE__, __LINE__, __func__, __VA_ARGS__)

#define RPMA_LOG_WARNING(...) \
	rpma_log(RPMA_LOG_LEVEL_WARNING, __FILE__, __LINE__, __func__, __VA_ARGS__)

#define RPMA_LOG_ERROR(...) \
	rpma_log(RPMA_LOG_LEVEL_ERROR, __FILE__, __LINE__, __func__, __VA_ARGS__)

#define RPMA_LOG_FATAL(...) \
	rpma_log(RPMA_LOG_LEVEL_FATAL, __FILE__, __LINE__, __func__, __VA_ARGS__)

#define RPMA_PRINTF(...) \
	rpma_log(RPMA_LOG_LEVEL_INFO, NULL, -1, NULL, __VA_ARGS__)

/*
 * rpma_log -- write messages either to syslog and to stderr
 * or call custom log function.
 * If level is set to RPMA_LOG_DISABLED,
 * the log message will neither be written to syslog nor to stderr.
 * Threshold is ignored if custom log function is provided via rpma_log_init().
 *
 * level - log level threshold.
 * file - name of the current source file.
 * line - current source line number.
 * func - current source function name.
 * format - format string to the message.
 */
void
rpma_log(enum rpma_log_level level, const char *file, const int line,
	const char *func, const char *format, ...)
	__attribute__((__format__(__printf__, 5, 6)));

/*
 * rpma_vlog -- Same as rpma_log except that instead of being called with variable number of
 * arguments it is called with an argument list as defined in stdarg.h
 *
 * level - Log level threshold.
 * file - Name of the current source file.
 * line - Current source line number.
 * func - Current source function name.
 * format - Format string to the message.
 * arg - printf arguments
 */
void
rpma_vlog(enum rpma_log_level level, const char *file, const int line,
	const char *func, const char *format, va_list arg);

#endif /* LOG_INTERNAL_H */
