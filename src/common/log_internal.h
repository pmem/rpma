// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020, Intel Corporation */

/*
 * \file
 * Logging interfaces used by librpma internally to produce logging information.
 */

#ifndef LIBRPMA_LOG_INTERNAL_H
#define LIBRPMA_LOG_INTERNAL_H
#include "librpma_log.h"


#define RPMA_NOTICELOG(...) \
	rpma_log(RPMA_LOG_NOTICE, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define RPMA_WARNLOG(...) \
	rpma_log(RPMA_LOG_WARN, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define RPMA_ERRLOG(...) \
	rpma_log(RPMA_LOG_ERROR, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define RPMA_PRINTF(...) \
	rpma_log(RPMA_LOG_NOTICE, NULL, -1, NULL, __VA_ARGS__)

/*
 * Write messages either to the syslog and to stderr
 * or call user defined log function.
 * If \c level is set to \c RPMA_LOG_DISABLED,
 * this log message won't be written to syslog and won't be written to stderr.
 *
 * \param level Log level threshold.
 * \param file Name of the current source file.
 * \param line Current source line number.
 * \param func Current source function name.
 * \param format Format string to the message.
 */
void rpma_log(enum rpma_log_level level, const char *file, const int line,
		const char *func, const char *format, ...) \
		__attribute__((__format__(__printf__, 5, 6)));

/*
 * Same as rpma_log except that instead of being called with variable number of
 * arguments it is called with an argument list as defined in stdarg.h
 *
 * \param level Log level threshold.
 * \param file Name of the current source file.
 * \param line Current source line number.
 * \param func Current source function name.
 * \param format Format string to the message.
 * \param ap printf arguments
 */
void rpma_vlog(enum rpma_log_level level, const char *file, const int line,
		const char *func, const char *format, va_list ap);

#endif /* LIBRPMA_LOG_INTERNAL_H */
