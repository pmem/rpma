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
	if (level <= Rpma_log_threshold[RPMA_LOG_THRESHOLD] && \
			NULL != Rpma_log_function) { \
		Rpma_log_function(level, __FILE__, __LINE__, __func__, \
				format, ##__VA_ARGS__); \
	}

/*
 * Set of macros that should be used as the primary API for logging.
 * Direct call to rpma_log shall be used only in exceptional, corner cases.
 */
#define RPMA_LOG_INFO(format, ...) \
	RPMA_LOG(RPMA_LOG_LEVEL_INFO, format "\n", ##__VA_ARGS__)

#define RPMA_LOG_NOTICE(format, ...) \
	RPMA_LOG(RPMA_LOG_LEVEL_NOTICE, format "\n", ##__VA_ARGS__)

#define RPMA_LOG_WARNING(format, ...) \
	RPMA_LOG(RPMA_LOG_LEVEL_WARNING, format "\n", ##__VA_ARGS__)

#define RPMA_LOG_ERROR(format, ...) \
	RPMA_LOG(RPMA_LOG_LEVEL_ERROR, format "\n", ##__VA_ARGS__)

#define RPMA_LOG_FATAL(format, ...) \
	RPMA_LOG(RPMA_LOG_LEVEL_FATAL, format "\n", ##__VA_ARGS__)

#define RPMA_LOG_ERRNO(format, ...) \
	RPMA_LOG(RPMA_LOG_LEVEL_ERROR, format "\n", ##__VA_ARGS__)

#define RPMA_LOG_ERRNO_FMT "%s failed: %s"

#define RPMA_LOG_ERROR_WITH_ERRNO(func, e) \
	RPMA_LOG_ERROR(RPMA_LOG_ERRNO_FMT, func, strerror(e))

#endif /* LIBRPMA_LOG_INTERNAL_H */
