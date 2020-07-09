/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2020, Intel Corporation
 */

/*
 * conn-test.c -- the conn unit tests hub
 */

#include "conn-test-common.h"

struct rpma_test_group groups[] = {
	RPMA_TEST_GROUP(tests_disconnect, NULL),
	RPMA_TEST_GROUP(tests_new, NULL),
	RPMA_TEST_GROUP(tests_next_completion, group_setup_next_completion),
	RPMA_TEST_GROUP(tests_next_event, NULL),
	RPMA_TEST_GROUP(tests_private_data, NULL),
	RPMA_TEST_GROUP(tests_read, group_setup_read),
	RPMA_TEST_GROUP(NULL, NULL),
};

int
main(int argc, char *argv[])
{
	return rpma_test_groups_run(groups);
}
