/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright 2020, Intel Corporation */

/*
 * log-test-to-syslog.h -- unit tests for logging to syslog
 */

#ifndef LOG_TEST_TO_SYSLOG_H
#define LOG_TEST_TO_SYSLOG_H

#include "log-test-common.h"
#include <syslog.h>

void syslog_mock_enable();
void syslog_mock_disable();
int setup_log(void **config_ptr);
int teardown_log(void **unused);

#endif /* LOG_TEST_TO_SYSLOG_H */
