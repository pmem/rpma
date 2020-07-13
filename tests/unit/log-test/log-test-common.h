/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright 2020, Intel Corporation */

/*
 * log-test-common.h -- unit tests of the log module - common tests
 */

#ifndef LOG_TEST_COMMON_H
#define LOG_TEST_COMMON_H

/*
 * syslog_set_threshold - test if it is possible to set all logging levels?
 */
void syslog_set_threshold(void **unused);

/*
 * syslog_set_threshold__invalid - do out of scope logging levels rejected?
 */
void syslog_set_threshold__invalid(void **unused);

/*
 * stderr_set_threshold - is it possible to set all logging to stderr levels?
 */
void stderr_set_threshold(void **unused);

/*
 * stderr_set_threshold__invalid -- do out of scope logging to stderr levels
 * rejected?
 */
void stderr_set_threshold__invalid(void **unused);

/*
 * log__out_of_threshold -- no output to stderr and syslog produced
 * for logging level out of threshold
 */
void log__out_of_threshold(void **unused);

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

#endif /* LOG_TEST_COMMON_H */
