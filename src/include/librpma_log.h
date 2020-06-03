// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2019-2020, Intel Corporation */
/*
 *   BSD LICENSE
 *
 *   Copyright (c) Intel Corporation.
 *   All rights reserved.
 *
 *   Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following conditions
 *   are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in
 *       the documentation and/or other materials provided with the
 *       distribution.
 *     * Neither the name of Intel Corporation nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

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
void rpma_log_set_level(enum rpma_log_level level);

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
void rpma_log_set_backtrace_level(enum rpma_log_level level);

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
void rpma_log_set_print_level(enum rpma_log_level level);

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
