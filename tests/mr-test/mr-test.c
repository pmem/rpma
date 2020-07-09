/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2020, Intel Corporation
 */

/*
 * mr-test.c -- the mr unit tests hub
 */

#include "mr-test-common.h"

struct test_group {
	const struct CMUnitTest * const tests;
	CMFixtureFunction group_setup;
};

struct test_group groups[] = {
	{tests_descriptor, NULL},
	{tests_read, group_setup_read},
	{tests_reg, NULL},
	{NULL, NULL}
};

int
main(int argc, char *argv[])
{
	int i = 0;
	int ret = 0;
	for (; groups[i].tests != NULL; ++i) {
		ret += cmocka_run_group_tests(
				groups[i].tests, groups[i].group_setup, NULL);
	}

	return ret;
}
