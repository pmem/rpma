/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright 2020, Intel Corporation */

/*
 * log-test-to-syslog.h -- unit tests for logging to syslog
 */

#ifndef LOG_TEST_TO_SYSLOG_H
#define LOG_TEST_TO_SYSLOG_H

#include "log-test-common.h"
#include <syslog.h>

/*
 * syslog_mock_enable() - enable mock behavior of syslog mocks
 */
void syslog_mock_enable();

/*
 * syslog_mock_disable() - disble mock behavior of syslog mocks
 */
void syslog_mock_disable();

/*
 * ensure that log is enabled & call setup_treshold to backup and setup
 * thresholds according to configuration
 */
int setup_log(void **config_ptr);

/*
 * teardown_log() - close log
 */
int teardown_log(void **unused);
#endif /* LOG_TEST_TO_SYSLOG_H */
