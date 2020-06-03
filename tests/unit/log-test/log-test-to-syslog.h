/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright 2020, Intel Corporation */

/*
 * log-test-to-syslog.h -- unit tests for logging to syslog
 */

#ifndef LOG_TEST_TO_SYSLOG_H
#define LOG_TEST_TO_SYSLOG_H

#include "log-test-common.h"
#include <syslog.h>

extern const struct CMUnitTest tests_log_to_syslog[];

/*
 * syslog_mock_enable() - enable mock behavior of syslog mocks
 */
void syslog_mock_enable();

/*
 * syslog_mock_disable() - disble mock behavior of syslog mocks
 */
void syslog_mock_disable();

/*
 * syslog_set_threshold - test if it is possible to set all logging levels?
 */
void syslog_set_threshold(void **unused);

/*
 * syslog_set_threshold__invalid - do out of scope logging levels rejected?
 */
void syslog_set_threshold__invalid(void **unused);

/*
 * log__to_syslog_out_of_threshold -- no output to syslog produced for logging
 * level out of threshold
 */
void log__to_syslog_out_of_threshold(void **unused);

/*
 * log__to_syslog -- successful logging to syslog with file related
 * information like file name, line number and function name
 */
void log__to_syslog(void **unused);

/*
 * log_to_syslog_no_file -- successful logging to syslog without file related
 * information
 */
void log__to_syslog_no_file(void **unused);

#endif /* LOG_TEST_TO_SYSLOG_H */
