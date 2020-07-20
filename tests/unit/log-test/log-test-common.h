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
 * level2name - log level to visible name string conversion
 */
const char *const level2name(rpma_log_level level);

/*
 * threshold tests config, setter/getter stored as well as backup threshold
 */
typedef struct {
	int (*set_threshold)(rpma_log_level);
	rpma_log_level (*get_threshold)(void);
	const rpma_log_level stderr_threshold;
	const rpma_log_level syslog_threshold;
}threshold_config;

/*
 * setup_threshold - set thresholds according to configuration
 */

int setup_threshold(void **config_ptr);

/*
 * set_threshold - is it possible to set all logging levels?
 */
void set_threshold(void **config_ptr);
/*
 * stderr_set_threshold__invalid - do out of scope logging to stderr levels
 * rejected?
 */
void
set_threshold__invalid(void **config_ptr);

/*
 * log__out_of_threshold -- no output to stderr or syslog produced for logging
 * level out of threshold
 */
void log__out_of_threshold(void **config_ptr);

/*
 * setup_default_threshold - setup and verify that default threshold
 * are accepted
 */
int setup_default_threshold(void **unused);

#endif /* LOG_TEST_COMMON_H */
