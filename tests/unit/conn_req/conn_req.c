// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020, Intel Corporation */

/*
 * conn_req.c -- a connection request unit test hub
 */

#include <infiniband/verbs.h>

#include "conn_req-common.h"
#include "cmocka_headers.h"
#include "conn_req.h"
#include "info.h"
#include "mocks-ibverbs.h"
#include "mocks-rdma_cm.h"
#include "rpma_err.h"
#include "test-common.h"

extern const struct CMUnitTest test_connect[];
extern const struct CMUnitTest test_delete[];
extern const struct CMUnitTest test_from_cm_event[];
extern const struct CMUnitTest test_new[];

struct rpma_test_group groups[] = {
	RPMA_TEST_GROUP(test_new, group_setup_conn_req),
	RPMA_TEST_GROUP(test_from_cm_event, group_setup_conn_req),
	RPMA_TEST_GROUP(test_delete, group_setup_conn_req),
	RPMA_TEST_GROUP(test_connect, group_setup_conn_req),
	RPMA_TEST_GROUP(NULL, NULL),
};

int
main(int argc, char *argv[])
{
	return rpma_test_groups_run(groups);
}
