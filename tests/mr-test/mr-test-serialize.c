/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2020, Intel Corporation
 */

/*
 * mr-serialize.c -- the memory region serialization unit tests
 *
 * APIs covered:
 * - rpma_mr_serialize_get_size()
 * - rpma_mr_serialize()
 * - rpma_mr_deserialize()
 * - rpma_mr_remote_delete()
 * - rpma_mr_remote_get_size()
 */

#include <stdlib.h>
#include <infiniband/verbs.h>

#include "mr-test-common.h"

/* setups & teardowns */

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

	*mr_ptr = mr;

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
	assert_int_equal(ret, RPMA_E_NOSUPP);
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

/* rpma_mr_serialiaze()/_deserialize() buffer alignment */

/*
 * test_serialize__buff_alignment - misc buff alignments
 */
static void
test_serialize__buff_alignment(void **mr_ptr)
{
	struct rpma_mr_local *mr = *mr_ptr;
	char buff_base[SERIALIZED_SIZE_EXP * 2];
	char *buff = NULL;
	char pattern[SERIALIZED_SIZE_EXP * 2];
	memset(pattern, 0xff, SERIALIZED_SIZE_EXP * 2);
	int ret = 0;

	/*
	 * Generate a miscellaneous output buffer alignment just to be sure the
	 * serialization algorithm does not prefer certain alignments.
	 */
	for (uintptr_t i = 0; i < SERIALIZED_SIZE_EXP; ++i) {
		memset(buff_base, 0xff, SERIALIZED_SIZE_EXP * 2);

		/* run test */
		buff = buff_base + i;
		ret = rpma_mr_serialize(mr, buff);

		/* verify the results */
		assert_int_equal(ret, 0);
		assert_memory_equal(buff, BUFF_EXP, SERIALIZED_SIZE_EXP);
		assert_memory_equal(buff_base, pattern, i);
		assert_memory_equal(
				buff_base + i + SERIALIZED_SIZE_EXP,
				pattern + i + SERIALIZED_SIZE_EXP,
				SERIALIZED_SIZE_EXP - i);
	}
}

/*
 * test_mr_deserialize__buff_alignment -- misc buff alignments
 */
static void
test_mr_deserialize__buff_alignment(void **unused)
{
	char buff_base[SERIALIZED_SIZE_EXP * 2];
	char *buff = NULL;
	char pattern[SERIALIZED_SIZE_EXP * 2];
	memset(pattern, 0xff, SERIALIZED_SIZE_EXP * 2);

	struct rpma_mr_remote *mr = NULL;
	size_t size = 0;
	int ret = 0;

	/* configure mock */
	will_return_always(__wrap__test_malloc, MOCK_OK);

	/*
	 * Generate a miscellaneous input buffer alignment just to be sure the
	 * deserialization algorithm does not prefer certain alignments.
	 */
	for (uintptr_t i = 0; i < SERIALIZED_SIZE_EXP; ++i) {
		memset(buff_base, 0xff, SERIALIZED_SIZE_EXP * 2);

		/* prepare a buffer contents */
		buff = buff_base + i;
		memcpy(buff, BUFF_EXP, SERIALIZED_SIZE_EXP);

		/* run test */
		ret = rpma_mr_deserialize(buff, SERIALIZED_SIZE_EXP, &mr);

		/* verify the results */
		assert_int_equal(ret, MOCK_OK);
		assert_non_null(mr);
		ret = rpma_mr_remote_get_size(mr, &size);
		assert_int_equal(ret, MOCK_OK);
		assert_int_equal(size, MOCK_SIZE);
		/*
		 * XXX When it will be possible verify addr, rkey, and placement
		 * using rpma_read or newly introduced getters.
		 */

		/* cleanup */
		ret = rpma_mr_remote_delete(&mr);
		assert_int_equal(ret, MOCK_OK);
		assert_null(mr);
	}
}

int
main(int argc, char *argv[])
{
	const struct CMUnitTest tests[] = {
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

		/* rpma_mr_serialiaze()/_deserialize() buffer alignment */
		cmocka_unit_test_setup_teardown(test_serialize__buff_alignment,
				setup__mr_reg_success,
				teardown__mr_dereg_success),
		cmocka_unit_test(test_mr_deserialize__buff_alignment),
	};

	return cmocka_run_group_tests(tests, NULL, NULL);
}
