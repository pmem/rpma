// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020, Intel Corporation */

/*
 * info-new.c -- unit tests of the info module
 *
 * APIs covered:
 * - rpma_info_new()
 * - rpma_info_delete()
 */

#include <stdlib.h>

#include "cmocka_headers.h"
#include "conn_req.h"
#include "info.h"
#include "librpma.h"
#include "info-common.h"
#include "mocks-rdma_cm.h"

#include <infiniband/verbs.h>

/*
 * new__addr_NULL -- NULL addr is not valid
 */
static void
new__addr_NULL(void **unused)
{
	/*
	 * NOTE: it is not allowed for info to allocate any resource before
	 * validating arguments.
	 */

	/* run test */
	struct rpma_info *info = NULL;
	int ret = rpma_info_new(NULL, "", RPMA_INFO_PASSIVE, &info);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
	assert_null(info);
}

/*
 * new__info_ptr_NULL -- NULL info_ptr is not valid
 */
static void
new__info_ptr_NULL(void **unused)
{
	/*
	 * NOTE: it is not allowed for info to allocate any resource before
	 * validating arguments.
	 */

	/* run test */
	int ret = rpma_info_new("", "", RPMA_INFO_PASSIVE, NULL);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * new__addr_port_info_ptr_NULL -- NULL addr, NULL port
 * and NULL info_ptr are not valid
 */
static void
new__addr_port_info_ptr_NULL(void **unused)
{
	/*
	 * NOTE: it is not allowed for info to allocate any resource before
	 * validating arguments.
	 */

	/* run test */
	int ret = rpma_info_new(NULL, NULL, RPMA_INFO_PASSIVE, NULL);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * new__getaddrinfo_EAGAIN_ACTIVE -- rdma_getaddrinfo() fails with
 * EAGAIN when side == RPMA_INFO_ACTIVE
 */
static void
new__getaddrinfo_EAGAIN_ACTIVE(void **unused)
{
	/*
	 * configure mocks:
	 * - NOTE: it is not allowed to call rdma_freeaddrinfo() if
	 * rdma_getaddrinfo() has failed.
	 */
	struct rdma_addrinfo_args get_args = {MOCK_VALIDATE, NULL};
	will_return(rdma_getaddrinfo, &get_args);
	expect_value(rdma_getaddrinfo, hints->ai_flags, 0);
	will_return(rdma_getaddrinfo, EAGAIN);
	will_return_maybe(__wrap__test_malloc, MOCK_OK);

	/* run test */
	struct rpma_info *info = NULL;
	int ret = rpma_info_new(MOCK_ADDR, MOCK_PORT, RPMA_INFO_ACTIVE,
			&info);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_null(info);
}

/*
 * new__getaddrinfo_EAGAIN_PASSIVE -- rdma_getaddrinfo() fails with
 * EAGAIN when side == RPMA_INFO_PASSIVE
 */
static void
new__getaddrinfo_EAGAIN_PASSIVE(void **unused)
{
	/*
	 * configure mocks:
	 * - NOTE: it is not allowed to call rdma_freeaddrinfo() if
	 * rdma_getaddrinfo() has failed.
	 */
	struct rdma_addrinfo_args get_args = {MOCK_VALIDATE, NULL};
	will_return(rdma_getaddrinfo, &get_args);
	expect_value(rdma_getaddrinfo, hints->ai_flags, RAI_PASSIVE);
	will_return(rdma_getaddrinfo, EAGAIN);
	will_return_maybe(__wrap__test_malloc, MOCK_OK);

	/* run test */
	struct rpma_info *info = NULL;
	int ret = rpma_info_new(MOCK_ADDR, MOCK_PORT, RPMA_INFO_PASSIVE,
			&info);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_null(info);
}

/*
 * new__malloc_ENOMEM -- malloc() fail with ENOMEM
 */
static void
new__malloc_ENOMEM(void **unused)
{
	/* configure mocks */
	struct rdma_addrinfo rai = {0};
	struct rdma_addrinfo_args args = {MOCK_PASSTHROUGH, &rai};
	will_return_maybe(rdma_getaddrinfo, &args);
	will_return_maybe(rdma_freeaddrinfo, &args);
	will_return(__wrap__test_malloc, ENOMEM);

	/* run test */
	struct rpma_info *info = NULL;
	int ret = rpma_info_new(MOCK_ADDR, MOCK_PORT, RPMA_INFO_PASSIVE,
			&info);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_NOMEM);
	assert_null(info);
}

/*
 * new__lifecycle -- happy day scenario
 */
static void
new__lifecycle(void **unused)
{
	/*
	 * configure mocks for rpma_info_new():
	 * NOTE: it is not allowed to call rdma_freeaddrinfo() if
	 * rdma_getaddrinfo() succeeded.
	 */
	struct rdma_addrinfo rai = {0};
	struct rdma_addrinfo_args args = {MOCK_VALIDATE, &rai};
	will_return(rdma_getaddrinfo, &args);
	expect_value(rdma_getaddrinfo, hints->ai_flags, RAI_PASSIVE);
	will_return(__wrap__test_malloc, MOCK_OK);

	/* run test - step 1 */
	struct rpma_info *info = NULL;
	int ret = rpma_info_new(MOCK_ADDR, MOCK_PORT, RPMA_INFO_PASSIVE,
			&info);

	/* verify the results */
	assert_int_equal(ret, MOCK_OK);
	assert_non_null(info);

	/*
	 * configure mocks for rpma_info_delete():
	 * NOTE: it is not allowed to call rdma_getaddrinfo() nor malloc() in
	 * rpma_info_delete().
	 */
	will_return(rdma_freeaddrinfo, &args);

	/* run test - step 2 */
	ret = rpma_info_delete(&info);

	/* verify the results */
	assert_int_equal(ret, MOCK_OK);
	assert_null(info);
}

/*
 * delete__info_ptr_NULL -- NULL info_ptr is not valid
 */
static void
delete__info_ptr_NULL(void **unused)
{
	/*
	 * NOTE: it is not allowed for info to allocate any resource before
	 * validating arguments.
	 */

	/* run test */
	int ret = rpma_info_delete(NULL);

	/* verify the result */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * delete__null_info -- valid NULL info
 */
static void
delete__null_info(void **unused)
{
	/*
	 * NOTE: it is not allowed for info to allocate any resource when
	 * quick-exiting.
	 */

	/* run test */
	struct rpma_info *info = NULL;
	int ret = rpma_info_delete(&info);

	/* verify the result */
	assert_int_equal(ret, MOCK_OK);
	assert_null(info);
}

int
main(int argc, char *argv[])
{
	const struct CMUnitTest tests[] = {
		/* rpma_info_new() unit tests */
		cmocka_unit_test(new__addr_NULL),
		cmocka_unit_test(new__info_ptr_NULL),
		cmocka_unit_test(new__addr_port_info_ptr_NULL),
		cmocka_unit_test(new__getaddrinfo_EAGAIN_ACTIVE),
		cmocka_unit_test(new__getaddrinfo_EAGAIN_PASSIVE),
		cmocka_unit_test(new__malloc_ENOMEM),

		/* rpma_info_delete() unit tests */
		cmocka_unit_test(delete__info_ptr_NULL),
		cmocka_unit_test(delete__null_info),

		/* rpma_info_new()/_delete() lifecycle */
		cmocka_unit_test(new__lifecycle),
	};

	return cmocka_run_group_tests(tests, NULL, NULL);
}
