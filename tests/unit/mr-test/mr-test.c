/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2020, Intel Corporation
 */

/*
 * mr-test.c -- the mr unit tests hub
 */

#include "mr-test-common.h"

extern const struct CMUnitTest tests_descriptor[];
extern const struct CMUnitTest tests_mr_read[];
extern const struct CMUnitTest tests_reg[];
extern const struct CMUnitTest tests_mr_write[];

static struct rpma_test_group groups[] = {
	RPMA_TEST_GROUP(tests_descriptor, NULL),
	RPMA_TEST_GROUP(tests_mr_read, group_setup_mr_read),
	RPMA_TEST_GROUP(tests_reg, NULL),
	RPMA_TEST_GROUP(tests_mr_write, NULL),
	RPMA_TEST_GROUP(NULL, NULL),
};

int
main(int argc, char *argv[])
{
	return rpma_test_groups_run(groups);
}
