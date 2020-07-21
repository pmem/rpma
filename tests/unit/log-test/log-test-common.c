// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020, Intel Corporation */

/*
 * log-test-common.c -- unit tests common module. Mock and common tests
 * implementation.
 *
 */

#include "log-test-common.h"
#include <string.h>

static const char *const levels2strings[] = {
	[RPMA_LOG_LEVEL_FATAL]	= "FATAL",
	[RPMA_LOG_LEVEL_ERROR]	= "ERROR",
	[RPMA_LOG_LEVEL_WARNING] = "WARNING",
	[RPMA_LOG_LEVEL_NOTICE]	= "NOTICE",
	[RPMA_LOG_LEVEL_INFO]	= "INFO",
	[RPMA_LOG_LEVEL_DEBUG]	= "DEBUG",
};

/*
 * level2string -- convert a log level to a string
 */
const char *const
level2string(rpma_log_level level)
{
	return levels2strings[level];
}

/*
 * setup_threshold -- set thresholds according to the configuration
 */
int
setup_threshold(void **config_ptr)
{
	struct threshold_config *config =
			(struct threshold_config *)*config_ptr;

	assert_int_equal(rpma_log_stderr_set_threshold(
				config->stderr_threshold), 0);
	assert_int_equal(rpma_log_syslog_set_threshold(
				config->syslog_threshold), 0);

	return 0;
}

/*
 * tests definitions
 */

/*
 * set_threshold__all -- set all possible threshold values
 */
void
set_threshold__all(void **config_ptr)
{
	struct threshold_config *config =
			(struct threshold_config *)*config_ptr;
	for (rpma_log_level level = RPMA_LOG_DISABLED;
		level <= RPMA_LOG_LEVEL_DEBUG; level++) {
		assert_int_equal(0, config->set_threshold(level));
		assert_int_equal(level, config->get_threshold());
	}
}

/*
 * stderr_set_threshold__invalid -- out of scope threshold shall be rejected
 */
void
set_threshold__invalid(void **config_ptr)
{
	struct threshold_config *config =
			(struct threshold_config *)*config_ptr;
	for (rpma_log_level level = RPMA_LOG_LEVEL_FATAL;
	    level <= RPMA_LOG_LEVEL_DEBUG; level++) {
		assert_int_equal(0, config->set_threshold(level));
		assert_int_equal(-1,
		    config->set_threshold(RPMA_LOG_LEVEL_DEBUG + 1));
		assert_int_equal(level, config->get_threshold());
		assert_int_equal(-1,
		    config->set_threshold(RPMA_LOG_DISABLED - 1));
		assert_int_equal(level, config->get_threshold());
	}
}

/*
 * set_threshold__default --default threshold are setup during default init?
 */
static void
set_threshold__default(void  **unused)
{
	assert_int_equal(0,
		rpma_log_syslog_set_threshold(RPMA_LOG_LEVEL_SYSLOG_DEFAULT));
	assert_int_equal(RPMA_LOG_LEVEL_SYSLOG_DEFAULT,
		rpma_log_syslog_get_threshold());
	assert_int_equal(0,
		rpma_log_stderr_set_threshold(RPMA_LOG_LEVEL_STDERR_DEFAULT));
	assert_int_equal(RPMA_LOG_LEVEL_STDERR_DEFAULT,
		rpma_log_stderr_get_threshold());
}

/*
 * common tests used in other test groups
 */

/*
 * log__out_of_threshold -- no output to stderr or syslog produced for logging
 * level out of threshold
 */
void
log__out_of_threshold(void **config_ptr)
{
	/* log to stderr and syslog enabled in setup() */
	struct threshold_config *config =
			(struct threshold_config *)*config_ptr;
	for (rpma_log_level level_min = RPMA_LOG_LEVEL_FATAL;
	    level_min <= RPMA_LOG_LEVEL_DEBUG; level_min++) {
		assert_int_equal(0, config->set_threshold(level_min));
		for (rpma_log_level level = level_min + 1;
			level <= RPMA_LOG_LEVEL_DEBUG; level++) {
			rpma_log(level, TEST_FILE_NAME, 1, TEST_FUNCTION_NAME,
				"%s", TEST_MESSAGE);
		}
	}
}

const struct CMUnitTest log_test_common[] = {
	cmocka_unit_test(set_threshold__all),
	cmocka_unit_test(set_threshold__invalid),
	cmocka_unit_test(set_threshold__default),

	cmocka_unit_test(NULL)
};
