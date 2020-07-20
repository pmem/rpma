// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020, Intel Corporation */

/*
 * log-test-common.c -- unit tests common module. Mock and common tests
 * implementation.
 *
 */

#include "log-test-common.h"
#include <string.h>

static const char *const level2names[] = {
	[RPMA_LOG_LEVEL_FATAL]	= "FATAL",
	[RPMA_LOG_LEVEL_ERROR]	= "ERROR",
	[RPMA_LOG_LEVEL_WARNING] = "WARNING",
	[RPMA_LOG_LEVEL_NOTICE]	= "NOTICE",
	[RPMA_LOG_LEVEL_INFO]	= "INFO",
	[RPMA_LOG_LEVEL_DEBUG]	= "DEBUG",
};

/*
 * level2name -- log level to visible name string conversion
 */
const char *const
level2name(rpma_log_level level)
{
	return level2names[level];
}

/*
 * setup_threshold -- set thresholds according to configuration
 */
int
setup_threshold(void **config_ptr)
{
	struct threshold_config *config =
			(struct threshold_config *)*config_ptr;
	int ret_stderr_set_threshold, ret_syslog_set_threshold;

	ret_stderr_set_threshold = rpma_log_stderr_set_threshold(
			config->stderr_threshold);
	assert_int_equal(ret_stderr_set_threshold, 0);

	ret_syslog_set_threshold = rpma_log_syslog_set_threshold(
			config->syslog_threshold);
	assert_int_equal(ret_syslog_set_threshold, 0);

	return ret_stderr_set_threshold || ret_syslog_set_threshold;
}

/*
 * set_threshold -- all available threshold shall be settable
 */
void
set_threshold(void **config_ptr)
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

/*
 * setup_default_threshold -- setup and verify that default threshold
 * are accepted
 */
int
setup_default_threshold(void **unused)
{
	assert_int_equal(0,
		rpma_log_syslog_set_threshold(RPMA_LOG_LEVEL_SYSLOG_DEFAULT));
	assert_int_equal(RPMA_LOG_LEVEL_SYSLOG_DEFAULT,
		rpma_log_syslog_get_threshold());
	assert_int_equal(0,
		rpma_log_stderr_set_threshold(RPMA_LOG_LEVEL_STDERR_DEFAULT));
	assert_int_equal(RPMA_LOG_LEVEL_STDERR_DEFAULT,
		rpma_log_stderr_get_threshold());
	return 0;
}
/*
 * init__default_treshold -- are we able to setup thresholds as it is done in
 * init function?
 */
void
init__default_treshold(void  **unused)
{
	;
}

const struct CMUnitTest log_test_common[] = {
	cmocka_unit_test_setup(init__default_treshold,
		setup_default_threshold),

	cmocka_unit_test(NULL)
};
