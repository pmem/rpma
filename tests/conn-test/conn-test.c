/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2020, Intel Corporation
 */

/*
 * conn-test.c -- the conn unit tests hub
 */

#include "conn-test-common.h"

struct test_group {
	const struct CMUnitTest * const tests;
	CMFixtureFunction group_setup;
};

struct test_group groups[] = {
	{tests_disconnect, NULL},
	{tests_new, NULL},
	{tests_next_completion, group_setup_next_completion},
	{tests_next_event, NULL},
	{tests_private_data, NULL},
	{tests_read, group_setup_read},
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
