/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2020, Intel Corporation
 */

/*
 * conn-test.c -- the conn unit tests hub
 */

#include "conn-test-common.h"

struct test_group {
	const char *name;
	const struct CMUnitTest * const tests;
	CMFixtureFunction group_setup;
};

struct test_group groups[] = {
	{"disconnect", tests_disconnect, NULL},
	{"new", tests_new, NULL},
	{"next_completion", tests_next_completion, group_setup_next_completion},
	{"next_event", tests_next_event, NULL},
	{"private_data", tests_private_data, NULL},
	{"read", tests_read, group_setup_read},
	{NULL, NULL, NULL}
};

int
main(int argc, char *argv[])
{
	int i = 0;
	int ret = 0;
	for (; groups[i].tests != NULL; ++i) {
		ret += _cmocka_run_group_tests(groups[i].name, groups[i].tests,
				cmocks_tests_num(groups[i].tests),
				groups[i].group_setup, NULL);
	}

	return ret;
}
