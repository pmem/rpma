/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright 2020, Intel Corporation */

/*
 * log-test-common.h -- unit tests of the log module - common tests
 */

#ifndef LOG_TEST_COMMON_H
#define LOG_TEST_COMMON_H

/*
 * test_set_level -- is it possible to set all logging levels?
 */
void test_set_level(void **unused);

/*
 * test_set_level_invalid -- do out of scope logging levels rejected?
 */
void test_set_level_invalid(void **unused);

/*
 * test_set_print_level -- is it possible to set all logging to stderr levels?
 */
void test_set_print_level(void **unused);

/*
 * test_set_print_level_invalid -- do out of scope logging to stderr levels rejected?
 */
void test_set_print_level_invalid(void **unused);

/*
 * test_log_out_of_threshold -- no output to stderr and syslog produced
 * for logging level out of threshold
 */
void test_log_out_of_threshold(void **unused);

/*
 * test_log_to_syslog -- message written to syslog
 */
void test_log_to_syslog(void **unused);

/*
 * test_log_to_syslog -- message (without file information) written to syslog
 */
void test_log_to_syslog_no_file(void **unused);

#endif /* LOG_TEST_COMMON_H */

