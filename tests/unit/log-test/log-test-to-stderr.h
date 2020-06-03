/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright 2020, Intel Corporation */

/*
 * log-test-to-stderr.h -- unit tests for logging to stderr
 */

#ifndef LOG_TEST_TO_STDERR_H
#define LOG_TEST_TO_STDERR_H
#include "log-test-common.h"

extern const struct CMUnitTest tests_log_to_stderr[];

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
 * log__to_stderr_out_of_threshold -- no output to stderr produced for logging
 * level out of threshold
 */
void log__to_stderr_out_of_threshold(void **unused);

#endif /* LOG_TEST_TO_STDERR_H */
