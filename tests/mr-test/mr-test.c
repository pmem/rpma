/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2020, Intel Corporation
 */

/*
 * mr-test.c -- the memory region unit tests
 */

#include <stdlib.h>
#include <infiniband/verbs.h>

#include "cmocka_headers.h"
#include "mr.h"
#include "librpma.h"
#include "rpma_err.h"

#define MOCK_PEER	(struct rpma_peer *)0x0AD0
#define MOCK_PTR	(void *)0x0001020304050607
#define MOCK_SIZE	(size_t)0x08090a0b0c0d0e0f
#define MOCK_USAGE	(int)(RPMA_MR_USAGE_READ_SRC | RPMA_MR_USAGE_READ_DST)
#define MOCK_PLT	(enum rpma_mr_plt)0x0AD4
#define MOCK_MR		(struct ibv_mr *)0x0AD5
#define MOCK_ERRNO	(int)(123)
#define MOCK_OK		(int)0

/*
 * Calculate an exptected value of required serialized memory region:
 * exp = raddr + size + rkey + plt
 *
 * Note: plt has two possible values so it fits perfectly into a single
 * uint8_t value.
 */
#define SERIALIZED_SIZE_EXP \
	(sizeof(uint64_t) * 2 + sizeof(uint32_t) + sizeof(uint8_t))

#define BUFF_EXP	"\x07\x06\x05\x04\x03\x02\x01\x00" \
			"\x0f\x0e\x0d\x0c\x0b\x0a\x09\x08" \
			"\x13\x12\x11\x10" \
			"\x01"

void *__real__test_malloc(size_t size);

/*
 * __wrap__test_malloc -- malloc() mock
 */
void *
__wrap__test_malloc(size_t size)
{
	errno = mock_type(int);

	if (errno)
		return NULL;

	return __real__test_malloc(size);
}

/*
 * rpma_peer_mr_reg -- a mock of rpma_peer_mr_reg()
 */
int
rpma_peer_mr_reg(struct rpma_peer *peer, struct ibv_mr **ibv_mr, void *addr,
	size_t length, int access)
{
	/*
	 * rpma_peer_mr_reg() and malloc() may be called in any order.
	 * If the first one fails, then the second one won't be called,
	 * so we cannot add cmocka's expects here.
	 * Otherwise, unconsumed expects would cause a test failure.
	 */
	*ibv_mr = mock_type(struct ibv_mr *);
	if (*ibv_mr == NULL) {
		Rpma_provider_error = mock_type(int);
		return RPMA_E_PROVIDER;
	}

	return 0;
}

/*
 * ibv_dereg_mr -- a mock of ibv_dereg_mr()
 */
int
ibv_dereg_mr(struct ibv_mr *mr)
{
	/*
	 * rpma_peer_mr_reg() and malloc() may be called in any order.
	 * If the first one fails, then the second one won't be called.
	 * ibv_dereg_mr() will be called in rpma_mr_reg() only if:
	 * 1) rpma_peer_mr_reg() succeeded and
	 * 2) malloc() failed.
	 * In the opposite case, when:
	 * 1) malloc() succeeded and
	 * 2) rpma_peer_mr_reg() failed,
	 * ibv_dereg_mr() will not be called,
	 * so we cannot add cmocka's expects here.
	 * Otherwise, unconsumed expects would cause a test failure.
	 */
	return mock_type(int); /* errno */
}

/* setups & teardowns */

/*
 * setup__mr_reg_success -- create a local memory registration object
 */
static int
setup__mr_reg_success(void **mr_ptr)
{
	/* configure mocks */
	will_return(__wrap__test_malloc, MOCK_OK);
	will_return(rpma_peer_mr_reg, MOCK_MR);

	/* run test */
	struct rpma_mr_local *mr = NULL;
	int ret = rpma_mr_reg(MOCK_PEER, MOCK_PTR, MOCK_SIZE,
				MOCK_USAGE, MOCK_PLT, &mr);

	/* verify the result */
	assert_int_equal(ret, MOCK_OK);
	assert_non_null(mr);

	*mr_ptr = mr;

	return 0;
}

/*
 * teardown__mr_dereg_success -- delete a local memory
 * registration object
 */
static int
teardown__mr_dereg_success(void **mr_ptr)
{
	struct rpma_mr_local *mr = *mr_ptr;

	/* configure mocks */
	will_return_maybe(ibv_dereg_mr, MOCK_OK);

	int ret = rpma_mr_dereg(&mr);

	/* verify the result */
	assert_int_equal(ret, MOCK_OK);
	assert_null(mr);

	*mr_ptr = NULL;

	return 0;
}

/*
 * setup__serialized_success -- register a local memory region's structure along
 * with a buffer with its serialized
 *
 * Note: In the wild when you will call rpma_mr_dereg() on the local memory
 * region structure the buffer with its serialized form is no longer useful.
 * For the sake of its test, it doesn't matter since the local memory region
 * does not have any backing-hardware resources.
 */
int
setup__serialized_success(void **buff_ptr)
{
	static char buff[SERIALIZED_SIZE_EXP] = {0};

	/* prepare local memory region structure */
	struct rpma_mr_local *mr = NULL;
	int ret = rpma_mr_reg(MOCK_PEER, MOCK_PTR, MOCK_SIZE, MOCK_USAGE,
			MOCK_PLT, &mr);

	/* verify the results */
	assert_int_equal(ret, MOCK_OK);
	assert_non_null(mr);

	/* serialize the local memory region */
	ret = rpma_mr_serialize(mr, buff);
	assert_int_equal(ret, MOCK_OK);

	*buff_ptr = buff;

	return 0;
}

/*
 * teardown__serialized -- small cleanup
 */
int
teardown__serialized(void **buff_ptr)
{
	*buff_ptr = NULL;

	return 0;
}

/*
 * setup__mr_remote -- create a remote memory region structure based on mocked
 * serialized memory region's data
 */
int
setup__mr_remote(void **mr_ptr)
{
	/* configure mock */
	will_return_maybe(__wrap__test_malloc, MOCK_OK);

	char buff[] = BUFF_EXP;

	/* deserialize the memory region as a remote access structure */
	struct rpma_mr_remote *mr = NULL;
	int ret = rpma_mr_deserialize(buff, SERIALIZED_SIZE_EXP, &mr);

	/* verify the results */
	assert_int_equal(ret, MOCK_OK);
	assert_non_null(mr);
	assert_memory_equal(buff, BUFF_EXP, SERIALIZED_SIZE_EXP);

	*mr_ptr = &mr;

	return 0;
}

/*
 * teardown__mr_remote -- delete the remote memory region's structure
 */
int
teardown__mr_remote(void **mr_ptr)
{
	struct rpma_mr_remote *mr = *mr_ptr;

	/* delete the remote memory region's structure */
	int ret = rpma_mr_remote_delete(&mr);
	assert_int_equal(ret, MOCK_OK);
	assert_null(mr);

	*mr_ptr = NULL;

	return 0;
}

/* rpma_mr_reg() unit tests */

/*
 * test_mr_reg__NULL_peer -- NULL peer is invalid
 */
static void
test_mr_reg__NULL_peer(void **unused)
{
	/* run test */
	struct rpma_mr_local *mr = NULL;
	int ret = rpma_mr_reg(NULL, MOCK_PTR, MOCK_SIZE,
				MOCK_USAGE, MOCK_PLT, &mr);

	/* verify the result */
	assert_int_equal(ret, RPMA_E_INVAL);
	assert_null(mr);
}

/*
 * test_mr_reg__NULL_ptr -- NULL ptr is invalid
 */
static void
test_mr_reg__NULL_ptr(void **unused)
{
	/* run test */
	struct rpma_mr_local *mr = NULL;
	int ret = rpma_mr_reg(MOCK_PEER, NULL, MOCK_SIZE,
				MOCK_USAGE, MOCK_PLT, &mr);

	/* verify the result */
	assert_int_equal(ret, RPMA_E_INVAL);
	assert_null(mr);
}

/*
 * test_mr_reg__NULL_mr -- NULL mr is invalid
 */
static void
test_mr_reg__NULL_mr(void **unused)
{
	/* run test */
	int ret = rpma_mr_reg(MOCK_PEER, MOCK_PTR, MOCK_SIZE,
				MOCK_USAGE, MOCK_PLT, NULL);

	/* verify the result */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * test_mr_reg__NULL_peer_ptr_mr -- NULL peer, ptr and mr are invalid
 */
static void
test_mr_reg__NULL_peer_ptr_mr(void **unused)
{
	/* run test */
	int ret = rpma_mr_reg(NULL, NULL, MOCK_SIZE,
				MOCK_USAGE, MOCK_PLT, NULL);

	/* verify the result */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * test_mr_reg__0_size -- size == 0 is invalid
 */
static void
test_mr_reg__0_size(void **unused)
{
	/* run test */
	struct rpma_mr_local *mr = NULL;
	int ret = rpma_mr_reg(MOCK_PEER, MOCK_PTR, 0,
				MOCK_USAGE, MOCK_PLT, &mr);

	/* verify the result */
	assert_int_equal(ret, RPMA_E_INVAL);
	assert_null(mr);
}

/*
 * test_mr_reg__failed_E_NOMEM -- rpma_mr_reg fails with ENOMEM
 */
static void
test_mr_reg__failed_E_NOMEM(void **unused)
{
	/* configure mocks */
	will_return(__wrap__test_malloc, ENOMEM);
	will_return_maybe(rpma_peer_mr_reg, MOCK_MR);
	will_return_maybe(ibv_dereg_mr, MOCK_OK);

	/* run test */
	struct rpma_mr_local *mr = NULL;
	int ret = rpma_mr_reg(MOCK_PEER, MOCK_PTR, MOCK_SIZE,
				MOCK_USAGE, MOCK_PLT, &mr);

	/* verify the result */
	assert_int_equal(ret, RPMA_E_NOMEM);
	assert_null(mr);
}

/*
 * test_mr_reg__failed_E_NOMEM -- rpma_mr_reg fails with ENOMEM
 */
static void
test_mr_reg__peer_mr_reg_failed_E_PROVIDER(void **unused)
{
	/* configure mocks */
	will_return(rpma_peer_mr_reg, NULL);
	will_return(rpma_peer_mr_reg, MOCK_ERRNO);
	will_return_maybe(__wrap__test_malloc, MOCK_OK);

	/* run test */
	struct rpma_mr_local *mr = NULL;
	int ret = rpma_mr_reg(MOCK_PEER, MOCK_PTR, MOCK_SIZE,
				MOCK_USAGE, MOCK_PLT, &mr);

	/* verify the result */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_null(mr);
}

/*
 * test_mr_reg_dereg__success -- happy day sccenario
 */
static void
test_mr_reg_dereg__success(void **unused)
{
	/*
	 * The whole thing is done by setup__mr_reg_success()
	 * and teardown__mr_dereg_success().
	 */
}

/* rpma_mr_dereg() unit tests */

/*
 * test_mr_dereg__NULL_pptr -- NULL pptr is invalid
 */
static void
test_mr_dereg__NULL_pptr(void **unused)
{
	/* run test */
	int ret = rpma_mr_dereg(NULL);

	/* verify the result */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * test_mr_dereg__NULL_ptr -- NULL ptr is OK
 */
static void
test_mr_dereg__NULL_ptr(void **unused)
{
	/* run test */
	struct rpma_mr_local *mr = NULL;
	int ret = rpma_mr_dereg(&mr);

	/* verify the result */
	assert_int_equal(ret, MOCK_OK);
}

/*
 * test_mr_dereg__failed_E_PROVIDER -- rpma_mr_dereg failed
 * with RPMA_E_PROVIDER
 */
static void
test_mr_dereg__failed_E_PROVIDER(void **unused)
{
	struct rpma_mr_local *mr;

	/* create a local memory registration object */
	(void) setup__mr_reg_success((void **)&mr);

	/* configure mocks */
	will_return_maybe(ibv_dereg_mr, MOCK_ERRNO);

	/* run test */
	int ret = rpma_mr_dereg(&mr);

	/* verify the result */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_null(mr);
}

/* rpma_mr_serialize_get_size() unit test */

/*
 * test_serialize_get_size__sanity - verify the returned value is as expected
 */
static void
test_serialize_get_size__sanity(void **unused)
{
	/* run test */
	assert_int_equal(rpma_mr_serialize_get_size(), SERIALIZED_SIZE_EXP);
}

/* rpma_mr_serialize() unit test */

/*
 * test_serialize__mr_NULL - NULL mr is invalid
 */
static void
test_serialize__mr_NULL(void **unused)
{
	char buff[SERIALIZED_SIZE_EXP] = {0};

	/* run test */
	int ret = rpma_mr_serialize(NULL, buff);

	/* verify the result */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * test_serialize__mr_NULL - NULL mr is invalid
 */
static void
test_serialize__buff_NULL(void **mr_ptr)
{
	struct rpma_mr_local *mr = *mr_ptr;

	/* run test */
	int ret = rpma_mr_serialize(mr, NULL);

	/* verify the result */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * test_serialize__mr_NULL_buff_NULL - NULL mr and NULL buff are invalid
 */
static void
test_serialize__mr_NULL_buff_NULL(void **unused)
{
	/* run test */
	int ret = rpma_mr_serialize(NULL, NULL);

	/* verify the result */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/* rpma_mr_deserialize() unit test */

/*
 * test_deserialize__buff_NULL - NULL buff is invalid
 */
static void
test_deserialize__buff_NULL(void **unused)
{
	/* run test */
	struct rpma_mr_remote *mr = NULL;
	int ret = rpma_mr_deserialize(NULL, SERIALIZED_SIZE_EXP, &mr);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
	assert_null(mr);
}

/*
 * test_deserialize__mr_ptr_NULL - NULL mr_ptr is invalid
 */
static void
test_deserialize__mr_ptr_NULL(void **unused)
{
	char buff[] =  BUFF_EXP;

	/* run test */
	int ret = rpma_mr_deserialize(buff, SERIALIZED_SIZE_EXP, NULL);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * test_deserialize__mr_ptr_NULL_buff_NULL - NULL mr_ptr and NULL buff are
 * invalid
 */
static void
test_deserialize__mr_ptr_NULL_buff_NULL(void **unused)
{
	/* run test */
	int ret = rpma_mr_deserialize(NULL, SERIALIZED_SIZE_EXP, NULL);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * test_deserialize__too_big_size - SERIALIZED_SIZE_EXP + 1 is invalid
 */
static void
test_deserialize__too_big_size(void **unused)
{
	char buff[] =  BUFF_EXP;

	/* run test */
	struct rpma_mr_remote *mr = NULL;
	int ret = rpma_mr_deserialize(buff, SERIALIZED_SIZE_EXP + 1, &mr);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_NOSUPP);
	assert_null(mr);
}

/*
 * test_deserialize__too_small_size - SERIALIZED_SIZE_EXP - 1 is invalid
 */
static void
test_deserialize__too_small_size(void **unused)
{
	char buff[] =  BUFF_EXP;

	/* run test */
	struct rpma_mr_remote *mr = NULL;
	int ret = rpma_mr_deserialize(buff, SERIALIZED_SIZE_EXP - 1, &mr);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_NOSUPP);
	assert_null(mr);
}

/*
 * test_deserialize__malloc_ENOMEM - malloc() fail with ENOMEM
 */
static void
test_deserialize__malloc_ENOMEM(void **unused)
{
	char buff[] =  BUFF_EXP;

	/* configure mock */
	will_return_maybe(__wrap__test_malloc, ENOMEM);

	/* run test */
	struct rpma_mr_remote *mr = NULL;
	int ret = rpma_mr_deserialize(buff, SERIALIZED_SIZE_EXP, &mr);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_NOMEM);
}

/*
 * test_deserialize__buff_plt_invalid - buff with invalid contents should be
 * detected as long as it breaks placement value
 */
static void
test_deserialize__buff_plt_invalid(void **unused)
{
	char buff_invalid[SERIALIZED_SIZE_EXP];
	memset(buff_invalid, 0xff, SERIALIZED_SIZE_EXP);

	/* configure mock */
	will_return_maybe(__wrap__test_malloc, MOCK_OK);

	/* run test */
	struct rpma_mr_remote *mr = NULL;
	int ret = rpma_mr_deserialize(buff_invalid, SERIALIZED_SIZE_EXP, &mr);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_UNKNOWN);
	assert_null(mr);
}

/* rpma_mr_remote_delete() unit test */

/*
 * test_remote_delete__mr_ptr_NULL - NULL mr_ptr is invalid
 */
static void
test_remote_delete__mr_ptr_NULL(void **unused)
{
	/* run test */
	int ret = rpma_mr_remote_delete(NULL);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * test_remote_delete__mr_NULL - NULL mr should exit quickly
 */
static void
test_remote_delete__mr_NULL(void **unused)
{
	/* run test */
	struct rpma_mr_remote *mr = NULL;
	int ret = rpma_mr_remote_delete(&mr);

	/* verify the results */
	assert_int_equal(ret, MOCK_OK);
	assert_null(mr);
}

/* rpma_mr_remote_get_size() unit test */

/*
 * test_remote_get_size__mr_ptr_NULL - NULL mr_ptr is invalid
 */
static void
test_remote_get_size__mr_ptr_NULL(void **unused)
{
	/* run test */
	size_t size = 0;
	int ret = rpma_mr_remote_get_size(NULL, &size);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
	assert_int_equal(size, 0);
}

/*
 * test_remote_get_size__size_NULL - NULL size is invalid
 */
static void
test_remote_get_size__size_NULL(void **mr_ptr)
{
	struct rpma_mr_remote *mr = *mr_ptr;

	/* run test */
	int ret = rpma_mr_remote_get_size(mr, NULL);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * test_remote_get_size__mr_ptr_NULL_size_NULL - NULL mr_ptr and NULL size are
 * invalid
 */
static void
test_remote_get_size__mr_ptr_NULL_size_NULL(void **unused)
{
	/* run test */
	int ret = rpma_mr_remote_get_size(NULL, NULL);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * test_remote_get_size__success - rpma_mr_remote_get_size() success
 */
static void
test_remote_get_size__success(void **mr_ptr)
{
	struct rpma_mr_remote *mr = *mr_ptr;

	/* verify the remote memory region correctness */
	size_t size = 0;
	int ret = rpma_mr_remote_get_size(mr, &size);
	assert_int_equal(ret, MOCK_OK);
	assert_int_equal(size, MOCK_SIZE);
}

int
main(int argc, char *argv[])
{
	const struct CMUnitTest tests[] = {
		/* rpma_mr_reg() unit tests */
		cmocka_unit_test(test_mr_reg__NULL_peer),
		cmocka_unit_test(test_mr_reg__NULL_ptr),
		cmocka_unit_test(test_mr_reg__NULL_mr),
		cmocka_unit_test(test_mr_reg__NULL_peer_ptr_mr),
		cmocka_unit_test(test_mr_reg__0_size),
		cmocka_unit_test(test_mr_reg__failed_E_NOMEM),
		cmocka_unit_test(test_mr_reg__peer_mr_reg_failed_E_PROVIDER),
		cmocka_unit_test_setup_teardown(test_mr_reg_dereg__success,
			setup__mr_reg_success, teardown__mr_dereg_success),

		/* rpma_mr_dereg() unit tests */
		cmocka_unit_test(test_mr_dereg__NULL_pptr),
		cmocka_unit_test(test_mr_dereg__NULL_ptr),
		cmocka_unit_test(test_mr_dereg__failed_E_PROVIDER),

		/* rpma_mr_serialize_get_size() unit test */
		cmocka_unit_test(test_serialize_get_size__sanity),

		/* rpma_mr_serialize() unit test */
		cmocka_unit_test(test_serialize__mr_NULL),
		cmocka_unit_test_setup_teardown(test_serialize__buff_NULL,
				setup__mr_reg_success,
				teardown__mr_dereg_success),
		cmocka_unit_test(test_serialize__mr_NULL_buff_NULL),

		/* rpma_mr_deserialize() unit test */
		cmocka_unit_test(test_deserialize__buff_NULL),
		cmocka_unit_test(test_deserialize__mr_ptr_NULL),
		cmocka_unit_test(test_deserialize__mr_ptr_NULL_buff_NULL),
		cmocka_unit_test(test_deserialize__too_big_size),
		cmocka_unit_test(test_deserialize__too_small_size),
		cmocka_unit_test(test_deserialize__malloc_ENOMEM),
		cmocka_unit_test(test_deserialize__buff_plt_invalid),

		/* rpma_mr_remote_delete() unit test */
		cmocka_unit_test(test_remote_delete__mr_ptr_NULL),
		cmocka_unit_test(test_remote_delete__mr_NULL),

		/* rpma_mr_remote_get_size() unit test */
		cmocka_unit_test(test_remote_get_size__mr_ptr_NULL),
		cmocka_unit_test_setup_teardown(test_remote_get_size__size_NULL,
				setup__mr_remote,
				teardown__mr_remote),
		cmocka_unit_test(test_remote_get_size__mr_ptr_NULL_size_NULL),
		cmocka_unit_test_setup_teardown(test_remote_get_size__success,
				setup__mr_remote,
				teardown__mr_remote),
	};

	return cmocka_run_group_tests(tests, NULL, NULL);
}
