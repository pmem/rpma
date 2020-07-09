/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2020, Intel Corporation
 */

/*
 * cmocka_headers.h -- include all headers required by cmocka
 */

#ifndef LIBRPMA_CMOCKA_H
#define LIBRPMA_CMOCKA_H 1

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

/*
 * rpma_test_group_size -- calculate the size of the NULL-terminated Cmocka
 * group
 */
static inline size_t
rpma_test_group_size(const struct CMUnitTest * const tests)
{
	size_t i = 0;
	while (tests[i].test_func != NULL)
		++i;

	return i - 1;
}

struct rpma_test_group {
	const char *name;
	const struct CMUnitTest * const tests;
	CMFixtureFunction setup;
};

#define RPMA_TEST_GROUP(f, setup) {#f, f, setup}

/*
 * rpma_test_groups_run -- run all provided RPMA test groups
 */
static inline int
rpma_test_groups_run(struct rpma_test_group *groups)
{
	int ret = 0;
	for (struct rpma_test_group *g = groups; g->tests != NULL; ++g) {
		ret += _cmocka_run_group_tests(g->name, g->tests,
				rpma_test_group_size(g->tests), g->setup,
				NULL);
	}

	return ret;
}

#endif /* LIBRPMA_CMOCKA_H */
