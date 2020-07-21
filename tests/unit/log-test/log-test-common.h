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


const char *const level2string(rpma_log_level level);

/*
 * Threshold configuration for all threshold related tests.
 * Also used in non-threshold related tests to sed initial thresholds values.
 * get/set_function are used to unify tests for sdrerr and syslog setter/getter
 * const threshold are used to preconfigure log with expected limits
 */
struct threshold_config {
	/* threshold set function */
	int (*set_threshold)(rpma_log_level);
	/* threshold get function */
	rpma_log_level (*get_threshold)(void);
	/* expected (forced during setup) threshold for stderr */
	const rpma_log_level stderr_threshold;
	/* expected (forced during setup) threshold for syslog */
	const rpma_log_level syslog_threshold;
};

int setup_threshold(void **config_ptr);
void set_threshold__all(void **config_ptr);
void set_threshold__invalid(void **config_ptr);
void log__out_of_threshold(void **config_ptr);

#endif /* LOG_TEST_COMMON_H */
