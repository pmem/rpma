/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2020, Intel Corporation
 */

/*
 * log-test-enabled.c -- unit tests of the log module
 * with logging enabled at startup
 */

#include <stdlib.h>
#include <syslog.h>
#include <string.h>
#include "cmocka_headers.h"
#include "../../src/common/log_internal.h"
#include "log-test-common.h"

/*
 * openlog -- openlog() mock
 */
void
openlog(const char *__ident, int __option, int __facility)
{
}

/*
 * closelog -- closelog() mock
 */
void
closelog(void)
{
}

/*
 * XXX missing tests to check initial logging levels:
 * #ifdef DEBUG
 *	rpma_log_set_level(RPMA_LOG_DEBUG);
 *	rpma_log_set_print_level(RPMA_LOG_WARN);
 * #else
 *	rpma_log_set_level(RPMA_LOG_WARN);
 *	rpma_log_set_print_level(RPMA_LOG_DISABLED);
 * #endif
 */

int
main(int argc, char *argv[])
{
	const struct CMUnitTest tests[] = {
		cmocka_unit_test(test_set_level),
		cmocka_unit_test(test_set_level_invalid),

		cmocka_unit_test(test_set_print_level),
		cmocka_unit_test(test_set_print_level_invalid),

		cmocka_unit_test(test_set_backtrace_level),
		cmocka_unit_test(test_set_backtrace_level_invalid),

		cmocka_unit_test(test_log_out_of_threshold),
		cmocka_unit_test(test_log_to_syslog),
		cmocka_unit_test(test_log_to_syslog_no_file),

	};

	int retVal = cmocka_run_group_tests(tests, NULL, NULL);
	return retVal;
}
