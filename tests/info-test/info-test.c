/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2020, Intel Corporation
 */

/*
 * info-test.c -- unit tests of the info module
 */

#include <stdlib.h>

#include "cmocka_headers.h"
#include "conn_req.h"
#include "info.h"
#include "librpma.h"

#include <infiniband/verbs.h>

#define MOCK_ADDR	"127.0.0.1"
#define MOCK_SERVICE	"1234"
#define MOCK_SRC_ADDR	(struct sockaddr *)0x0ADD
#define MOCK_DST_ADDR	(struct sockaddr *)0x0ADE

#define MOCK_PASSTHROUGH	0
#define MOCK_VALIDATE		1

#define MOCK_OK		0

static const struct rdma_cm_id Cmid_zero = {0};

struct rdma_addrinfo_args {
	int validate_params;
	struct rdma_addrinfo *res;
};

/*
 * rdma_getaddrinfo -- rdma_getaddrinfo() mock
 */
int
rdma_getaddrinfo(const char *node, const char *service,
		const struct rdma_addrinfo *hints, struct rdma_addrinfo **res)
{
	struct rdma_addrinfo_args *args =
				mock_type(struct rdma_addrinfo_args *);
	if (args->validate_params == MOCK_VALIDATE) {
		assert_string_equal(node, MOCK_ADDR);
		assert_string_equal(service, MOCK_SERVICE);
		check_expected(hints->ai_flags);
	}

	*res = args->res;
	if (*res != NULL) {
		if (hints->ai_flags & RAI_PASSIVE) {
			(*res)->ai_flags |=RAI_PASSIVE;
		}
	}

	if (*res != NULL)
		return 0;

	errno = mock_type(int);

	return -1;
}

/*
 * rdma_freeaddrinfo -- rdma_freeaddrinfo() mock
 */
void
rdma_freeaddrinfo(struct rdma_addrinfo *res)
{
	struct rdma_addrinfo_args *args =
				mock_type(struct rdma_addrinfo_args *);
	if (args->validate_params == MOCK_VALIDATE)
		assert_ptr_equal(res, args->res);
}

/*
 * rdma_resolve_addr -- rdma_resolve_addr() mock
 * Note: CM ID is not modified.
 */
int
rdma_resolve_addr(struct rdma_cm_id *id, struct sockaddr *src_addr,
		struct sockaddr *dst_addr, int timeout_ms)
{
	check_expected_ptr(id);
	check_expected_ptr(src_addr);
	check_expected_ptr(dst_addr);
	check_expected(timeout_ms);

	errno = mock_type(int);
	if (errno)
		return -1;

	return 0;
}

/*
 * rdma_bind_addr -- rdma_bind_addr() mock
 * Note: CM ID is not modified.
 */
int
rdma_bind_addr(struct rdma_cm_id *id, struct sockaddr *addr)
{
	check_expected_ptr(id);
	check_expected_ptr(addr);

	errno = mock_type(int);
	if (errno)
		return -1;

	return 0;
}

void *__real__test_malloc(size_t size);

/*
 * __wrap__test_malloc -- malloc() mock
 */
void *
__wrap__test_malloc(size_t size)
{
	int err = mock_type(int);

	if (err) {
		errno = err;
		return NULL;
	}

	return __real__test_malloc(size);
}

/*
 * info_new_test_addr_NULL -- NULL addr is not valid
 */
static void
info_new_test_addr_NULL(void **unused)
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
 * info_new_test_info_ptr_NULL -- NULL info_ptr is not valid
 */
static void
info_new_test_info_ptr_NULL(void **unused)
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
 * info_new_test_addr_service_info_ptr_NULL -- NULL addr, NULL service
 * and NULL info_ptr are not valid
 */
static void
info_new_test_addr_service_info_ptr_NULL(void **unused)
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
 * info_new_test_getaddrinfo_EAGAIN -- rdma_getaddrinfo() fails with
 * EAGAIN
 */
static void
info_new_test_getaddrinfo_EAGAIN(void **unused)
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
	int ret = rpma_info_new(MOCK_ADDR, MOCK_SERVICE, RPMA_INFO_PASSIVE,
			&info);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_int_equal(rpma_err_get_provider_error(), EAGAIN);
	assert_null(info);
}

/*
 * info_new_test_malloc_ENOMEM -- malloc() fail with ENOMEM
 */
static void
info_new_test_malloc_ENOMEM(void **unused)
{
	/* configure mocks */
	struct rdma_addrinfo rai = {0};
	struct rdma_addrinfo_args args = {MOCK_PASSTHROUGH, &rai};
	will_return_maybe(rdma_getaddrinfo, &args);
	will_return_maybe(rdma_freeaddrinfo, &args);
	will_return(__wrap__test_malloc, ENOMEM);

	/* run test */
	struct rpma_info *info = NULL;
	int ret = rpma_info_new(MOCK_ADDR, MOCK_SERVICE, RPMA_INFO_PASSIVE,
			&info);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_NOMEM);
	assert_null(info);
}

/*
 * info_test_lifecycle -- happy day scenario
 */
static void
info_test_lifecycle(void **unused)
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
	int ret = rpma_info_new(MOCK_ADDR, MOCK_SERVICE, RPMA_INFO_PASSIVE,
			&info);

	/* verify the results */
	assert_int_equal(ret, MOCK_OK);
	assert_non_null(info);
// TG	assert_int_equal(RPMA_INFO_PASSIVE, info->side);
/*
 * TG
 */
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
 * info_delete_test_info_ptr_NULL -- NULL info_ptr is not valid
 */
static void
info_delete_test_info_ptr_NULL(void **unused)
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
 * info_delete_test_null_info -- valid NULL info
 */
static void
info_delete_test_null_info(void **unused)
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

/*
 * all the resources used between info_setup_* and info_teardown
 */
struct info_state {
	struct rdma_addrinfo rai;
	struct rpma_info *info;
};

/*
 * info_setup_passive -- prepare a valid rpma_info object (passive side)
 */
static int
info_setup_passive(void **info_state_ptr)
{
	/*
	 * configure mocks for rpma_info_new():
	 * NOTE: it is not allowed to call rdma_freeaddrinfo() if
	 * rdma_getaddrinfo() succeeded.
	 */
	static struct info_state istate;
	memset(&istate, 0, sizeof(istate));
	istate.rai.ai_src_addr = MOCK_SRC_ADDR;
	istate.rai.ai_dst_addr = MOCK_DST_ADDR;
	struct rdma_addrinfo_args args = {MOCK_VALIDATE, &istate.rai};
	will_return(rdma_getaddrinfo, &args);
	expect_value(rdma_getaddrinfo, hints->ai_flags, RAI_PASSIVE);
	will_return(__wrap__test_malloc, MOCK_OK);

	int ret = rpma_info_new(MOCK_ADDR, MOCK_SERVICE, RPMA_INFO_PASSIVE,
			&istate.info);
	assert_int_equal(ret, 0);
	assert_non_null(istate.info);

	*info_state_ptr = &istate;

	return 0;
}

/*
 * info_setup_active -- prepare a valid rpma_info object (active side)
 */
static int
info_setup_active(void **info_state_ptr)
{
	/*
	 * configure mocks for rpma_info_new():
	 * NOTE: it is not allowed to call rdma_freeaddrinfo() if
	 * rdma_getaddrinfo() succeeded.
	 */
	static struct info_state istate;
	memset(&istate, 0, sizeof(istate));
	istate.rai.ai_src_addr = MOCK_SRC_ADDR;
	istate.rai.ai_dst_addr = MOCK_DST_ADDR;
	struct rdma_addrinfo_args args = {MOCK_VALIDATE, &istate.rai};
	will_return(rdma_getaddrinfo, &args);
	expect_value(rdma_getaddrinfo, hints->ai_flags, 0);
	will_return(__wrap__test_malloc, MOCK_OK);

	int ret = rpma_info_new(MOCK_ADDR, MOCK_SERVICE, RPMA_INFO_ACTIVE,
			&istate.info);
	assert_int_equal(ret, 0);
	assert_non_null(istate.info);

	*info_state_ptr = &istate;
/*
 * TG shall we test here that INFO ACTIVE
 */
	return 0;
}

/*
 * info_teardown -- delete the rpma_info object (either active or passive side)
 */
static int
info_teardown(void **info_state_ptr)
{
	struct info_state *istate = *info_state_ptr;

	/*
	 * configure mocks for rdma_freeaddrinfo():
	 * NOTE: it is not allowed to call rdma_freeaddrinfo() nor malloc() in
	 * rpma_info_delete().
	 */
	struct rdma_addrinfo_args args = {MOCK_VALIDATE, &istate->rai};
	will_return(rdma_freeaddrinfo, &args);

	/* teardown */
	int ret = rpma_info_delete(&istate->info);
	assert_int_equal(ret, MOCK_OK);
	assert_null(istate->info);

	return 0;
}

/*
 * info_resolve_addr_test_id_NULL -- NULL id is invalid
 */
static void
info_resolve_addr_test_id_NULL(void **info_state_ptr)
{
	struct info_state *istate = *info_state_ptr;

	/* run test */
	int ret = rpma_info_assign_addr(istate->info, NULL);

	/* verify the result */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * info_resolve_addr_test_info_NULL -- NULL info is invalid
 */
static void
info_resolve_addr_test_info_NULL(void **unused)
{
	/* run test */
	struct rdma_cm_id cmid = {0};
	int ret = rpma_info_assign_addr(NULL, &cmid);

	/* verify the result */
	assert_int_equal(ret, RPMA_E_INVAL);
	assert_int_equal(memcmp(&cmid, &Cmid_zero, sizeof(cmid)), 0);
}

/*
 * info_resolve_addr_test_id_info_NULL -- NULL id and info are invalid
 */
static void
info_resolve_addr_test_id_info_NULL(void **unused)
{
	/* run test */
	int ret = rpma_info_assign_addr(NULL, NULL);

	/* verify the result */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * info_resolve_addr_test_resolve_addr_EAGAIN -- rdma_resolve_addr() fails
 * with EAGAIN
 */
static void
info_resolve_addr_test_resolve_addr_EAGAIN(void **info_state_ptr)
{
	struct info_state *istate = *info_state_ptr;

	/* configure mocks */
	struct rdma_cm_id cmid = {0};
	expect_value(rdma_resolve_addr, id, &cmid);
	expect_value(rdma_resolve_addr, src_addr, MOCK_SRC_ADDR);
	expect_value(rdma_resolve_addr, dst_addr, MOCK_DST_ADDR);
	expect_value(rdma_resolve_addr, timeout_ms, RPMA_DEFAULT_TIMEOUT);
	will_return(rdma_resolve_addr, EAGAIN);

	/* run test */
	int ret = rpma_info_assign_addr(istate->info, &cmid);

	/* verify the result */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_int_equal(rpma_err_get_provider_error(), EAGAIN);
	assert_int_equal(memcmp(&cmid, &Cmid_zero, sizeof(cmid)), 0);
}

/*
 * info_resolve_addr_test_success -- happy day scenario
 */
static void
info_resolve_addr_test_success(void **info_state_ptr)
{
	struct info_state *istate = *info_state_ptr;

	/* configure mocks */
	struct rdma_cm_id cmid = {0};
	expect_value(rdma_resolve_addr, id, &cmid);
	expect_value(rdma_resolve_addr, src_addr, MOCK_SRC_ADDR);
	expect_value(rdma_resolve_addr, dst_addr, MOCK_DST_ADDR);
	expect_value(rdma_resolve_addr, timeout_ms, RPMA_DEFAULT_TIMEOUT);
	will_return(rdma_resolve_addr, MOCK_OK);

	/* run test */
	int ret = rpma_info_assign_addr(istate->info, &cmid);

	/* verify the result */
	assert_int_equal(ret, MOCK_OK);
	assert_int_equal(memcmp(&cmid, &Cmid_zero, sizeof(cmid)), 0);
}

/*
 * info_bind_addr_test_id_NULL -- NULL id is invalid
 */
static void
info_bind_addr_test_id_NULL(void **info_state_ptr)
{
	struct info_state *istate = *info_state_ptr;

	/* run test */
	int ret = rpma_info_assign_addr(istate->info, NULL);

	/* verify the result */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * info_bind_addr_test_info_NULL -- NULL info is invalid
 */
static void
info_bind_addr_test_info_NULL(void **unused)
{
	/* run test */
	struct rdma_cm_id cmid = {0};
	int ret = rpma_info_assign_addr(NULL, &cmid);

	/* verify the result */
	assert_int_equal(ret, RPMA_E_INVAL);
	assert_int_equal(memcmp(&cmid, &Cmid_zero, sizeof(cmid)), 0);
}

/*
 * info_bind_addr_test_id_info_NULL -- NULL id and info are invalid
 */
static void
info_bind_addr_test_id_info_NULL(void **unused)
{
	/* run test */
	int ret = rpma_info_assign_addr(NULL, NULL);

	/* verify the result */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * info_bind_addr_test_bind_addr_EAGAIN -- rpma_info_assign_addr() fails
 * with EAGAIN
 */
static void
info_bind_addr_test_bind_addr_EAGAIN(void **info_state_ptr)
{
	struct info_state *istate = *info_state_ptr;

	/* configure mocks */
	struct rdma_cm_id cmid = {0};
	expect_value(rdma_bind_addr, id, &cmid);
	expect_value(rdma_bind_addr, addr, MOCK_SRC_ADDR);
	will_return(rdma_bind_addr, EAGAIN);

	/* run test */
	int ret = rpma_info_assign_addr(istate->info, &cmid);

	/* verify the result */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_int_equal(rpma_err_get_provider_error(), EAGAIN);
	assert_int_equal(memcmp(&cmid, &Cmid_zero, sizeof(cmid)), 0);
}

/*
 * info_bind_addr_test_success -- happy day scenario
 */
static void
info_bind_addr_test_success(void **info_state_ptr)
{
	struct info_state *istate = *info_state_ptr;

	/* configure mocks */
	struct rdma_cm_id cmid = {0};
	expect_value(rdma_bind_addr, id, &cmid);
	expect_value(rdma_bind_addr, addr, MOCK_SRC_ADDR);
	will_return(rdma_bind_addr, MOCK_OK);

	/* run test */
	int ret = rpma_info_assign_addr(istate->info, &cmid);

	/* verify the result */
	assert_int_equal(ret, MOCK_OK);
	assert_int_equal(memcmp(&cmid, &Cmid_zero, sizeof(cmid)), 0);
}

int
main(int argc, char *argv[])
{
	const struct CMUnitTest tests[] = {

		/* rpma_info_new() unit tests */
		cmocka_unit_test(info_new_test_addr_NULL),
		cmocka_unit_test(info_new_test_info_ptr_NULL),
		cmocka_unit_test(
			info_new_test_addr_service_info_ptr_NULL),
		cmocka_unit_test(info_new_test_getaddrinfo_EAGAIN),
		cmocka_unit_test(info_new_test_malloc_ENOMEM),

		/* rpma_info_new()/_delete() lifecycle */
		cmocka_unit_test(info_test_lifecycle),

		/* rpma_info_delete() unit tests */
		cmocka_unit_test(info_delete_test_info_ptr_NULL),
		cmocka_unit_test(info_delete_test_null_info),

		/* rpma_info_resolve_addr() unit tests */
		cmocka_unit_test_setup_teardown(
				info_resolve_addr_test_id_NULL,
				info_setup_active, info_teardown),
		cmocka_unit_test(info_resolve_addr_test_info_NULL),
		cmocka_unit_test(info_resolve_addr_test_id_info_NULL),
		cmocka_unit_test_setup_teardown(
				info_resolve_addr_test_resolve_addr_EAGAIN,
				info_setup_active, info_teardown),

		cmocka_unit_test_setup_teardown(
				info_resolve_addr_test_success,
				info_setup_active, info_teardown),


		/* rpma_info_assign_addr() unit tests */
		cmocka_unit_test_setup_teardown(
				info_bind_addr_test_id_NULL,
				info_setup_passive, info_teardown),
		cmocka_unit_test(info_bind_addr_test_info_NULL),
		cmocka_unit_test(info_bind_addr_test_id_info_NULL),
		cmocka_unit_test_setup_teardown(
				info_bind_addr_test_bind_addr_EAGAIN,
				info_setup_passive, info_teardown),

		cmocka_unit_test_setup_teardown(
				info_bind_addr_test_success,
				info_setup_passive, info_teardown),
	};

	return cmocka_run_group_tests(tests, NULL, NULL);
}
