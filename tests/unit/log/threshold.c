// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020, Intel Corporation */

/*
 * threshold.c -- rpma_log_[get/set]_threshold unit tests
 */

#include <stdlib.h>

#include "cmocka_headers.h"
#include "log_internal.h"
#include "log_default.h"
#include "librpma.h"

#define INVALID_THRESHOLD_MOCK	- 1
#define INVALID_LEVEL_MOCK	- 2

/*
 * set_threshold__threshold_invalid -- set invalid threshold
 */
void
set_threshold__threshold_invalid(void **unused)
{
	int ret = rpma_log_set_threshold(INVALID_THRESHOLD_MOCK,
			RPMA_LOG_DISABLED);

	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * set_threshold__level_invalid -- set invalid level
 */
void
set_threshold__level_invalid(void **unused)
{
	int ret = rpma_log_set_threshold(RPMA_LOG_THRESHOLD,
			INVALID_LEVEL_MOCK);

	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * set_threshold__threshold_level_invalid -- set invalid threshold and level
 */
void
set_threshold__threshold_level_invalid(void **unused)
{
	int ret = rpma_log_set_threshold(INVALID_THRESHOLD_MOCK,
			INVALID_LEVEL_MOCK);

	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * set_threshold__success -- happy day scenario
 */
void
set_threshold__success(void **unused)
{
	int ret = rpma_log_set_threshold(RPMA_LOG_THRESHOLD, RPMA_LOG_DISABLED);

	assert_int_equal(ret, 0);
}

/*
 * get_threshold__threshold_invalid -- set invalid threshold
 */
void
get_threshold__threshold_invalid(void **unused)
{
	rpma_log_level level;
	int ret = rpma_log_get_threshold(INVALID_THRESHOLD_MOCK, &level);

	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * get_threshold__level_invalid -- NULL level is invalid
 */
void
get_threshold__level_invalid(void **unused)
{
	int ret = rpma_log_get_threshold(RPMA_LOG_THRESHOLD, NULL);

	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * get_threshold__threshold_level_invalid -- set invalid threshold and level
 */
void
get_threshold__threshold_level_invalid(void **unused)
{
	int ret = rpma_log_get_threshold(INVALID_THRESHOLD_MOCK, NULL);

	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * get_threshold__success -- happy day scenario
 */
void
get_threshold__success(void **unused)
{
	rpma_log_level level;
	int ret = rpma_log_get_threshold(RPMA_LOG_THRESHOLD, &level);

	assert_int_equal(ret, 0);
}

int
main(int argc, char *argv[])
{
	const struct CMUnitTest tests[] = {
		/* rpma_log_set_threshold() unit tests */
		cmocka_unit_test(set_threshold__threshold_invalid),
		cmocka_unit_test(set_threshold__level_invalid),
		cmocka_unit_test(set_threshold__threshold_level_invalid),
		cmocka_unit_test(set_threshold__success),

		/* rpma_log_get_threshold() unit tests */
		cmocka_unit_test(get_threshold__threshold_invalid),
		cmocka_unit_test(get_threshold__level_invalid),
		cmocka_unit_test(get_threshold__threshold_level_invalid),
		cmocka_unit_test(get_threshold__success),
	};

	return cmocka_run_group_tests(tests, NULL, NULL);
}
