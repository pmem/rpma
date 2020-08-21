// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020, Intel Corporation */

/*
 * librpma_constructor.c -- library constructor test
 */

#include "cmocka_headers.h"

void librpma_init(void);
void librpma_fini(void);

/*
 * init__success -- librpma_init() unit test
 */
static void
init__success(void **unused)
{
	expect_function_call(rpma_log_init);
	librpma_init();
}

/*
 * fini__success -- librpma_fini() unit test
 */
static void
fini__success(void **unused)
{
	expect_function_call(rpma_log_fini);
	librpma_fini();
}

int
main(int argc, char *argv[])
{
	const struct CMUnitTest tests[] = {
		cmocka_unit_test(init__success),
		cmocka_unit_test(fini__success),
	};

	return cmocka_run_group_tests(tests, NULL, NULL);
}
