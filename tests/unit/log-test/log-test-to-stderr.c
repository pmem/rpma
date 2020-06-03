// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020, Intel Corporation */

/*
 * log-test-default.c -- unit tests of the log module with the default
 * log function (logging to stderr and syslog)
 */

#include <stdlib.h>
#include <string.h>
#include "log-test-to-stderr.h"

typedef struct set_threshold_state {
	int (*set_func)(rpma_log_level);
	rpma_log_level (*get_func)(void);
	int (*set_func_to_disable)(rpma_log_level);
	rpma_log_level (*get_func_to_disable)(void);
	rpma_log_level oryginal_state_disable;
	rpma_log_level oryginal_state;
}set_threshold_state;

int
setup_threshold(void **p_state)
{
	set_threshold_state *state = (set_threshold_state*) *p_state;
	if(state->get_func)
		state->oryginal_state = state->get_func();
	if(state->get_func_to_disable)
		state->oryginal_state_disable = state->get_func_to_disable();
	if(state->set_func_to_disable)
		assert_int_equal(0,
			state->set_func_to_disable(RPMA_LOG_DISABLED));
	return 0;
}
int
teardown_threshold(void **p_state)
{
	set_threshold_state *state = (set_threshold_state*) *p_state;
	if (state->get_func_to_disable && state->set_func_to_disable)
		assert_int_equal(0,
				state->set_func_to_disable(
						state->oryginal_state_disable));
	if (state->get_func && state->set_func)
		assert_int_equal(0,
				state->set_func(state->oryginal_state));
	return 0;
}

/*
 * log__to_stderr_out_of_threshold -- no output to stderr produced for logging
 * level out of threshold
 */
void
log__to_stderr_out_of_threshold(void **p_state)
{
	set_threshold_state *state = (set_threshold_state*) *p_state;
	for (rpma_log_level level_min = RPMA_LOG_LEVEL_FATAL;
	    level_min <= RPMA_LOG_LEVEL_DEBUG; level_min++) {
		assert_int_equal(0, state->set_func(level_min));
		for (rpma_log_level level = level_min + 1;
			level <= RPMA_LOG_LEVEL_DEBUG; level++) {
			rpma_log(level, TEST_FILE_NAME, 1, TEST_FUNCTION_NAME,
				"%s", TEST_MESSAGE);
		}
	}
}

/*
 * stderr_set_threshold__invalid -- do out of scope logging to stderr levels
 * rejected?
 */
void
stderr_set_threshold__invalid(void **unused)
{
	rpma_log_level level = RPMA_LOG_LEVEL_DEBUG;
	rpma_log_level level_org = rpma_log_stderr_get_threshold();
	level++;
	assert_int_equal(-1, rpma_log_stderr_set_threshold(level));
	assert_int_equal(level_org, rpma_log_stderr_get_threshold());
	level = RPMA_LOG_DISABLED;
	level--;
	assert_int_equal(-1, rpma_log_stderr_set_threshold(level));
	assert_int_equal(level_org, rpma_log_stderr_get_threshold());
}

/*
 * stderr_set_threshold - is it possible to set all logging to stderr levels?
 */
void
stderr_set_threshold(void **unused)
{
	for (rpma_log_level level = RPMA_LOG_DISABLED;
		level <= RPMA_LOG_LEVEL_DEBUG; level++) {
		assert_int_equal(0, rpma_log_stderr_set_threshold(level));
		assert_int_equal(level, rpma_log_stderr_get_threshold());
	}
}

set_threshold_state state = {
		rpma_log_stderr_set_threshold,
		rpma_log_stderr_get_threshold,
		rpma_log_syslog_set_threshold,
		rpma_log_syslog_get_threshold,
		RPMA_LOG_DISABLED,RPMA_LOG_DISABLED
};

const struct CMUnitTest tests_log_to_stderr[] = {

/*
 * threshold setters/getters tests
 */
	cmocka_unit_test_setup(stderr_set_threshold__invalid,
		setup_default_threshold),
	cmocka_unit_test_setup(stderr_set_threshold,
		setup_default_threshold),

/*
 * logging with levels out of threshold
 */
	cmocka_unit_test_prestate_setup_teardown(
			log__to_stderr_out_of_threshold,
			setup_threshold,
			teardown_threshold, &state),
	cmocka_unit_test(NULL)
};
