/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2020, Intel Corporation
 */

/*
 * conn-test.c -- the conn unit tests hub
 */

#include "conn-test-common.h"

extern const struct CMUnitTest tests_disconnect[];
extern const struct CMUnitTest tests_get_event_fd[];
extern const struct CMUnitTest tests_new[];
extern const struct CMUnitTest tests_next_completion[];
extern const struct CMUnitTest tests_next_event[];
extern const struct CMUnitTest tests_private_data[];
extern const struct CMUnitTest tests_read[];
extern const struct CMUnitTest tests_write[];

struct rpma_test_group groups[] = {
	RPMA_TEST_GROUP(tests_disconnect, NULL),
	RPMA_TEST_GROUP(tests_get_event_fd, group_setup_get_event_fd),
	RPMA_TEST_GROUP(tests_new, NULL),
	RPMA_TEST_GROUP(tests_next_completion, group_setup_next_completion),
	RPMA_TEST_GROUP(tests_next_event, NULL),
	RPMA_TEST_GROUP(tests_private_data, NULL),
	RPMA_TEST_GROUP(tests_read, group_setup_read),
	RPMA_TEST_GROUP(tests_write, group_setup_write),
	RPMA_TEST_GROUP(NULL, NULL),
};

int
main(int argc, char *argv[])
{
	return rpma_test_groups_run(groups);
}
