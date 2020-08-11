/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright 2020, Intel Corporation */

/*
 * log_internal.h -- internal logging interfaces used by the librpma.
 */

#ifndef LIBRPMA_LOG_INTERNAL_H
#define LIBRPMA_LOG_INTERNAL_H

#include "librpma_log.h"

/* pointer to the logging function */
extern log_function *Rpma_log_function;

/* threshold levels */
extern rpma_log_level Rpma_log_threshold[RPMA_LOG_THRESHOLD_MAX];

void rpma_log_init();

void rpma_log_fini();

#define RPMA_LOG(level, format, ...) \
	if (level <= Rpma_log_threshold[RPMA_LOG_THRESHOLD_PRIMARY] && \
			NULL != Rpma_log_function) { \
		Rpma_log_function(level, __FILE__, __LINE__, __func__, \
				format, ##__VA_ARGS__); \
	}

/*
 * Set of macros that should be used as the primary API for logging.
 * Direct call to rpma_log shall be used only in exceptional, corner cases.
 */
#define RPMA_LOG_NOTICE(format, ...) \
	RPMA_LOG(RPMA_LOG_LEVEL_NOTICE, format "\n", ##__VA_ARGS__)

#define RPMA_LOG_WARNING(format, ...) \
	RPMA_LOG(RPMA_LOG_LEVEL_WARNING, format "\n", ##__VA_ARGS__)

#define RPMA_LOG_ERROR(format, ...) \
	RPMA_LOG(RPMA_LOG_LEVEL_ERROR, format "\n", ##__VA_ARGS__)

#define RPMA_LOG_FATAL(format, ...) \
	RPMA_LOG(RPMA_LOG_LEVEL_FATAL, format "\n", ##__VA_ARGS__)

/*
 * rpma_log - call either the default or a custom log function.
 *
 * The default log function write messages to syslog and to stderr.
 * The message flow can be controlled with help of threshold setters function:
 * rpma_log_syslog_set_threshold and rpma_log_stderr_set_threshold.
 * Threshold set to RPMA_LOG_DISABLED disable particular message destination
 * (syslog/stderr).
 *
 * All log messages are redirected to a custom log function if it is provided
 * by rmpa_log_init() function. No messages are produced to syslog and stderr.
 * A custom log function receives all log messages and it is not affected by
 * the thresholds described above.
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

/*
 * Default thresholds for logging levels
 */
#ifdef DEBUG
#define RPMA_LOG_THRESHOLD_PRIMARY_DEFAULT RPMA_LOG_LEVEL_DEBUG
#define RPMA_LOG_THRESHOLD_SECONDARY_DEFAULT RPMA_LOG_LEVEL_WARNING
#else
#define RPMA_LOG_THRESHOLD_PRIMARY_DEFAULT RPMA_LOG_LEVEL_WARNING
#define RPMA_LOG_THRESHOLD_SECONDARY_DEFAULT RPMA_LOG_DISABLED
#endif

#endif /* LIBRPMA_LOG_INTERNAL_H */
