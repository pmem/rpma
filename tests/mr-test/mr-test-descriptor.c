/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2020, Intel Corporation
 */

/*
 * mr-descriptor.c -- the memory region serialization unit tests
 *
 * APIs covered:
 * - rpma_mr_get_descriptor()
 * - rpma_mr_remote_from_descriptor()
 * - rpma_mr_remote_delete()
 * - rpma_mr_remote_get_size()
 */

#include <stdlib.h>
#include <infiniband/verbs.h>

#include "mr-test-common.h"

#define MR_DESC_SIZE sizeof(struct rpma_mr_descriptor)

static const struct rpma_mr_descriptor Desc_exp = DESC_EXP;

/* setups & teardowns */

/*
 * setup__mr_remote -- create a remote memory region structure based on a
 * pre-prepared memory region's descriptor
 */
int
setup__mr_remote(void **mr_ptr)
{
	/* configure mock */
	will_return_maybe(__wrap__test_malloc, MOCK_OK);

	/*
	 * create a remote memory structure based on a pre-prepared descriptor
	 */
	struct rpma_mr_remote *mr = NULL;
	int ret = rpma_mr_remote_from_descriptor(&Desc_exp, &mr);

	/* verify the results */
	assert_int_equal(ret, MOCK_OK);
	assert_non_null(mr);

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

/* rpma_mr_get_descriptor() unit test */

/*
 * test_get_descriptor__mr_NULL - NULL mr is invalid
 */
static void
test_get_descriptor__mr_NULL(void **unused)
{
	struct rpma_mr_descriptor desc = {0};

	/* run test */
	int ret = rpma_mr_get_descriptor(NULL, &desc);

	/* verify the result */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * test_get_descriptor__desc_NULL - NULL desc is invalid
 */
static void
test_get_descriptor__desc_NULL(void **mr_ptr)
{
	struct rpma_mr_local *mr = *mr_ptr;

	/* run test */
	int ret = rpma_mr_get_descriptor(mr, NULL);

	/* verify the result */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * test_get_descriptor__mr_NULL_desc_NULL - NULL mr and NULL desc are invalid
 */
static void
test_get_descriptor__mr_NULL_desc_NULL(void **unused)
{
	/* run test */
	int ret = rpma_mr_get_descriptor(NULL, NULL);

	/* verify the result */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/* rpma_mr_remote_from_descriptor() unit test */

/*
 * test_remote_from_descriptor__desc_NULL - NULL desc is invalid
 */
static void
test_remote_from_descriptor__desc_NULL(void **unused)
{
	/* run test */
	struct rpma_mr_remote *mr = NULL;
	int ret = rpma_mr_remote_from_descriptor(NULL, &mr);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
	assert_null(mr);
}

/*
 * test_remote_from_descriptor__mr_ptr_NULL - NULL mr_ptr is invalid
 */
static void
test_remote_from_descriptor__mr_ptr_NULL(void **unused)
{
	/* run test */
	int ret = rpma_mr_remote_from_descriptor(&Desc_exp, NULL);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * test_remote_from_descriptor__mr_ptr_NULL_desc_NULL - NULL mr_ptr and NULL
 * desc are invalid
 */
static void
test_remote_from_descriptor__mr_ptr_NULL_desc_NULL(void **unused)
{
	/* run test */
	int ret = rpma_mr_remote_from_descriptor(NULL, NULL);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * test_remote_from_descriptor__malloc_ENOMEM - malloc() fail with ENOMEM
 */
static void
test_remote_from_descriptor__malloc_ENOMEM(void **unused)
{
	/* configure mock */
	will_return_maybe(__wrap__test_malloc, ENOMEM);

	/* run test */
	struct rpma_mr_remote *mr = NULL;
	int ret = rpma_mr_remote_from_descriptor(&Desc_exp, &mr);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_NOMEM);
}

/*
 * test_remote_from_descriptor__desc_plt_invalid - buff with invalid contents
 * should be detected as long as it breaks placement value
 */
static void
test_remote_from_descriptor__buff_plt_invalid(void **unused)
{
	struct rpma_mr_descriptor desc_invalid = {0};
	memset(&desc_invalid, 0xff, sizeof(struct rpma_mr_descriptor));

	/* configure mock */
	will_return_maybe(__wrap__test_malloc, MOCK_OK);

	/* run test */
	struct rpma_mr_remote *mr = NULL;
	int ret = rpma_mr_remote_from_descriptor(&desc_invalid, &mr);

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
 * test_remote_get_size__size_NULL - NULL size pointer is invalid
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
 * test_remote_get_size__mr_ptr_NULL_size_NULL - NULL mr_ptr and NULL size
 * pointer are invalid
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

/* rpma_mr_serialiaze()/_remote_from_descriptor() buffer alignment */

/*
 * test_get_descriptor__desc_alignment - misc desc alignments
 */
static void
test_get_descriptor__desc_alignment(void **pprestate)
{
	struct prestate *prestate = *pprestate;
	struct rpma_mr_local *mr = prestate->mr;

	char buff_base[MR_DESC_SIZE * 2];
	char pattern[MR_DESC_SIZE * 2];
	memset(pattern, 0xff, MR_DESC_SIZE * 2);
	struct rpma_mr_descriptor *desc = NULL;
	int ret = 0;

	/*
	 * Generate a miscellaneous output descriptor alignment just to be sure
	 * the implementation does not prefer certain alignments.
	 */
	for (uintptr_t i = 0; i < MR_DESC_SIZE; ++i) {
		memset(buff_base, 0xff, MR_DESC_SIZE * 2);

		/* run test */
		desc = (struct rpma_mr_descriptor *)(buff_base + i);
		ret = rpma_mr_get_descriptor(mr, desc);

		/* verify the results */
		assert_int_equal(ret, 0);
		assert_memory_equal(desc, &Desc_exp, MR_DESC_SIZE);
		assert_memory_equal(buff_base, pattern, i);
		assert_memory_equal(
				buff_base + i + MR_DESC_SIZE,
				pattern + i + MR_DESC_SIZE,
				MR_DESC_SIZE - i);
	}
}

/*
 * test_remote_from_descriptor__desc_alignment -- misc desc alignments
 */
static void
test_remote_from_descriptor__desc_alignment(void **unused)
{
	char buff_base[MR_DESC_SIZE * 2];
	char pattern[MR_DESC_SIZE * 2];
	memset(pattern, 0xff, MR_DESC_SIZE * 2);

	struct rpma_mr_descriptor *desc = NULL;
	struct rpma_mr_remote *mr = NULL;
	size_t size = 0;
	int ret = 0;

	/* configure mock */
	will_return_always(__wrap__test_malloc, MOCK_OK);

	/*
	 * Generate a miscellaneous input descriptor alignment just to be sure
	 * the implementation does not prefer certain alignments.
	 */
	for (uintptr_t i = 0; i < MR_DESC_SIZE; ++i) {
		memset(buff_base, 0xff, MR_DESC_SIZE * 2);

		/* prepare a buffer contents */
		desc = (struct rpma_mr_descriptor *)(buff_base + i);
		memcpy(desc, &Desc_exp, MR_DESC_SIZE);

		/* run test */
		ret = rpma_mr_remote_from_descriptor(desc, &mr);

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
	struct prestate prestate =
		{RPMA_MR_USAGE_READ_SRC, IBV_ACCESS_REMOTE_READ, NULL};

	const struct CMUnitTest tests[] = {
		/* rpma_mr_get_descriptor() unit test */
		cmocka_unit_test(test_get_descriptor__mr_NULL),
		cmocka_unit_test_prestate_setup_teardown(
			test_get_descriptor__desc_NULL,
			setup__reg_success,
			teardown__dereg_success,
			&prestate),
		cmocka_unit_test(test_get_descriptor__mr_NULL_desc_NULL),

		/* rpma_mr_remote_from_descriptor() unit test */
		cmocka_unit_test(test_remote_from_descriptor__desc_NULL),
		cmocka_unit_test(test_remote_from_descriptor__mr_ptr_NULL),
		cmocka_unit_test(
			test_remote_from_descriptor__mr_ptr_NULL_desc_NULL),
		cmocka_unit_test(test_remote_from_descriptor__malloc_ENOMEM),
		cmocka_unit_test(test_remote_from_descriptor__buff_plt_invalid),

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

		/*
		 * rpma_mr_get_descriptor()/rpma_mr_remote_from_descriptor()
		 * buffer alignment
		 */
		cmocka_unit_test_prestate_setup_teardown(
			test_get_descriptor__desc_alignment,
			setup__reg_success,
			teardown__dereg_success,
			&prestate),
		cmocka_unit_test(test_remote_from_descriptor__desc_alignment),
	};

	return cmocka_run_group_tests(tests, NULL, NULL);
}
