/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright 2020, Intel Corporation */

/*
 * log-test-common.h -- log unit tests common module
 */

#ifndef LOG_TEST_COMMON_H
#define LOG_TEST_COMMON_H

#include "cmocka_headers.h"
#include "log_internal.h"

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)
#define TEST_FILE_NAME "foo_bar.c"
#define TEST_FUNCTION_NAME "foo_bar()"
#define TEST_LINE_NO 1
#define TEST_MESSAGE "Message"

/*
 * threshold tests config, setter/getter functions as well as expected
 * thresholds for stderr and syslog
 */
struct threshold_config {
	int (*set_threshold)(rpma_log_level);
	rpma_log_level (*get_threshold)(void);
	const rpma_log_level stderr_threshold;
	const rpma_log_level syslog_threshold;
};

const char *const level2name(rpma_log_level level);
int setup_threshold(void **config_ptr);
void set_threshold(void **config_ptr);
void set_threshold__invalid(void **config_ptr);
void log__out_of_threshold(void **config_ptr);
int setup_default_threshold(void **unused);

#endif /* LOG_TEST_COMMON_H */
