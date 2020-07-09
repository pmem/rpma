/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2020, Intel Corporation
 */

/*
 * mr-test.c -- the mr unit tests hub
 */

#include "mr-test-common.h"

struct test_group {
	const char *name;
	const struct CMUnitTest * const tests;
	CMFixtureFunction group_setup;
};

static struct test_group groups[] = {
	{"descriptor", tests_descriptor, NULL},
	{"read", tests_mr_read, group_setup_mr_read},
	{"reg", tests_reg, NULL},
	{NULL, NULL, NULL}
};

int
main(int argc, char *argv[])
{
	int ret = 0;
	for (int i = 0; groups[i].tests != NULL; ++i) {
		ret += _cmocka_run_group_tests(groups[i].name, groups[i].tests,
				cmocks_tests_num(groups[i].tests),
				groups[i].group_setup, NULL);
	}

	return ret;
}
