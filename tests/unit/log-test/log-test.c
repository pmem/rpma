/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2020, Intel Corporation
 */

/*
 * log-test.c -- the log unit tests hub
 */

#include "log-test-common.h"
#include "log-test-to-syslog.h"

extern const struct CMUnitTest log_test_common[];
extern const struct CMUnitTest log_test_to_stderr[];
extern const struct CMUnitTest log_test_to_syslog[];
extern const struct CMUnitTest log_test_to_custom_function[];

struct rpma_test_group groups[] = {
	RPMA_TEST_GROUP(log_test_common, NULL),
	RPMA_TEST_GROUP(log_test_to_stderr, NULL),
	RPMA_TEST_GROUP(log_test_to_syslog, NULL),
	RPMA_TEST_GROUP(log_test_to_custom_function, NULL),
	RPMA_TEST_GROUP(NULL, NULL),
};

int
main(int argc, char *argv[])
{
	return rpma_test_groups_run(groups);
}
