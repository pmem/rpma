/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright 2020, Intel Corporation */

/*
 * log_internal.h -- internal logging interfaces used by the librpma.
 */

#ifndef LIBRPMA_LOG_INTERNAL_H
#define LIBRPMA_LOG_INTERNAL_H

#include "librpma_log.h"

/*
 * Set of macros that should be used as the primary API for logging.
 * Direct call to rpma_log shall be used only in exceptional, corner cases.
 */
#define RPMA_LOG_NOTICE(format, ...) \
	rpma_log(RPMA_LOG_LEVEL_NOTICE, __FILE__, __LINE__, __func__, \
		format "\n", ##__VA_ARGS__)

#define RPMA_LOG_WARNING(format, ...) \
	rpma_log(RPMA_LOG_LEVEL_WARNING, __FILE__, __LINE__, __func__, \
		format "\n", ##__VA_ARGS__)

#define RPMA_LOG_ERROR(format, ...) \
	rpma_log(RPMA_LOG_LEVEL_ERROR, __FILE__, __LINE__, __func__, \
		format "\n", ##__VA_ARGS__)

#define RPMA_LOG_FATAL(format, ...) \
	rpma_log(RPMA_LOG_LEVEL_FATAL, __FILE__, __LINE__, __func__, \
		format "\n", ##__VA_ARGS__)

#define RPMA_PRINTF(format, ...) \
	rpma_log(RPMA_LOG_LEVEL_INFO, NULL, -1, NULL, format, ##__VA_ARGS__)

/*
 * rpma_log - write messages either to syslog and to stderr or call
 * custom log function.
 *
 * Message flow to syslog and stderr can be controlled with help of
 * threshold setters function: rpma_log_syslog_set_threshold and
 * rpma_log_stderr_set_threshold.
 *
 * Threshold set to RPMA_LOG_DISABLED disable particular message destination
 * (syslog/stderr).
 *
 * All log messages are redirected to a custom log function if it is provided
 * by rmpa_log_init() function. No messages are produced to syslog and stderr.
 *
 * Parameters are as follow:
 * level - log level.
 * file_name - name of the current source file. NULL value indicate that there
 * is also neither line_no nor function name provided.
 * line_no - current source line number.
 * function_name - current source function name. Must not be NULL if file_name
 * is given.
 * message_format - printf() like message string.
 *
 * ASSUMPTIONS
 * - file_name == NULL || function_name != NULL
 * - format != NULL
 */
__attribute__((__format__(__printf__, 5, 6)))
void rpma_log(rpma_log_level level, const char *file_name, const int line_no,
	const char *function_name, const char *message_format, ...);

#endif /* LIBRPMA_LOG_INTERNAL_H */
