/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright 2020, Intel Corporation */

/*
 * log-test-common.h -- unit tests common module
 */

#ifndef LOG_TEST_COMMON_H
#define LOG_TEST_COMMON_H
#include "cmocka_headers.h"
#include "common/log_internal.h"

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)
#define TEST_FILE_NAME "ala_ma_kota.c"
#define TEST_FUNCTION_NAME "foo_bar()"
#define TEST_LINE_NO 1
#define TEST_MESSAGE "Message"

static const char *const expected_level_names[] = {
	[RPMA_LOG_LEVEL_FATAL]	= "FATAL",
	[RPMA_LOG_LEVEL_ERROR]	= "ERROR",
	[RPMA_LOG_LEVEL_WARNING] = "WARNING",
	[RPMA_LOG_LEVEL_NOTICE]	= "NOTICE",
	[RPMA_LOG_LEVEL_INFO]	= "INFO",
	[RPMA_LOG_LEVEL_DEBUG]	= "DEBUG",
};

/*
 * setup_default_threshold - setup and verify that default threshold
 * are accepted
 */
int setup_default_threshold(void **unused);

#endif /* LOG_TEST_COMMON_H */
