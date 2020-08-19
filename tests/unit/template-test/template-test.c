// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020, Intel Corporation */

/*
 * template-test.c -- a template test
 */

#include "cmocka_headers.h"

/*
 * template_test - test case that does nothing and succeeds
 */
static void
template_test(void **state)
{
	(void) state; /* unused */
}

int
main(int argc, char *argv[])
{
	const struct CMUnitTest tests[] = {
		cmocka_unit_test(template_test)
	};

	return cmocka_run_group_tests(tests, NULL, NULL);
}
