// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020, Intel Corporation */

/*
 * init_fini.c -- rpma_log_default_init/_fini() unit tests
 */

#include <syslog.h>

#include "cmocka_headers.h"
#include "log_default.h"

/*
 * init__normal -- happy day scenario
 */
void
init__normal(void  **unused)
{
	/* configure mocks */
	expect_string(openlog, __ident, "rpma");
	expect_value(openlog, __option, LOG_PID);
	expect_value(openlog, __facility, LOG_LOCAL7);

	/* run test */
	rpma_log_default_init();
}

/*
 * fini__normal -- happy day scenario
 */
void
fini__normal(void  **unused)
{
	/* configure mocks */
	expect_function_call(closelog);

	/* run test */
	rpma_log_default_fini();
}

int
main(int argc, char *argv[])
{
	const struct CMUnitTest tests[] = {
		cmocka_unit_test(init__normal),
		cmocka_unit_test(fini__normal)
	};

	return cmocka_run_group_tests(tests, NULL, NULL);
}
