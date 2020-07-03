/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright 2020, Intel Corporation */

/*
 * log_internal.h -- internal logging interfaces used by the librpma.
 */

#ifndef LOG_INTERNAL_H
#define LOG_INTERNAL_H
#include "librpma_log.h"

#define RPMA_NOTICELOG(...) \
	rpma_log(RPMA_LOG_NOTICE, __FILE__, __LINE__, __func__, __VA_ARGS__)

#define RPMA_WARNLOG(...) \
	rpma_log(RPMA_LOG_WARN, __FILE__, __LINE__, __func__, __VA_ARGS__)

#define RPMA_ERRLOG(...) \
	rpma_log(RPMA_LOG_ERROR, __FILE__, __LINE__, __func__, __VA_ARGS__)

#define RPMA_FATALLOG(...) \
	rpma_log(RPMA_LOG_FATAL, __FILE__, __LINE__, __func__, __VA_ARGS__)

#define RPMA_PRINTF(...) \
	rpma_log(RPMA_LOG_NOTICE, NULL, -1, NULL, __VA_ARGS__)

/*
 * Write messages either to the syslog and to stderr
 * or call user defined log function.
 * If level is set to RPMA_LOG_DISABLED,
 * this log message will neither be written to syslog nor to stderr.
 *
 * level - Log level threshold.
 * file - Name of the current source file.
 * line - Current source line number.
 * func - Current source function name.
 * format - Format string to the message.
 */
void rpma_log(enum rpma_log_level level, const char *file, const int line,
	const char *func, const char *format, ...)
	__attribute__((__format__(__printf__, 5, 6)));

/*
 * Same as rpma_log except that instead of being called with variable number of
 * arguments it is called with an argument list as defined in stdarg.h
 *
 * level - Log level threshold.
 * file - Name of the current source file.
 * line - Current source line number.
 * func - Current source function name.
 * format - Format string to the message.
 * arg - printf arguments
 */
void rpma_vlog(enum rpma_log_level level, const char *file, const int line,
	const char *func, const char *format, va_list arg);

#endif /* LOG_INTERNAL_H */
