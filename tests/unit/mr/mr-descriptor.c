// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020, Intel Corporation */
/* Copyright 2021, Fujitsu */

/*
 * mr-descriptor.c -- the memory region serialization unit tests
 *
 * APIs covered:
 * - rpma_mr_get_descriptor_size()
 * - rpma_mr_get_descriptor()
 * - rpma_mr_remote_from_descriptor()
 * - rpma_mr_remote_delete()
 * - rpma_mr_remote_get_size()
 */

#include <stdlib.h>
#include <infiniband/verbs.h>

#include "mocks-rpma-peer.h"
#include "mr-common.h"
#include "test-common.h"

/* rpma_mr_get_descriptor_size() unit test */

/*
 * get_descriptor_size__mr_NULL - NULL mr is invalid
 */
static void
get_descriptor_size__mr_NULL(void **unused)
{
	size_t desc_size;

	/* run test */
	int ret = rpma_mr_get_descriptor_size(NULL, &desc_size);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * get_descriptor_size__desc_size_NULL - NULL desc_size is invalid
 */
static void
get_descriptor_size__desc_size_NULL(void **pprestate)
{
	struct prestate *prestate = *pprestate;
	struct rpma_mr_local *mr = prestate->mr;

	/* run test */
	int ret = rpma_mr_get_descriptor_size(mr, NULL);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * get_descriptor_size__mr_desc_size_NULL - NULL mr and NULL desc_size
 * are invalid
 */
static void
get_descriptor_size__mr_desc_size_NULL(void **unused)
{
	/* run test */
	int ret = rpma_mr_get_descriptor_size(NULL, NULL);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * get_descriptor_size__success - happy day scenario
 */
static void
get_descriptor_size__success(void **pprestate)
{
	struct prestate *prestate = *pprestate;
	struct rpma_mr_local *mr = prestate->mr;
	size_t desc_size;

	/* run test */
	int ret = rpma_mr_get_descriptor_size(mr, &desc_size);

	/* verify the results */
	assert_int_equal(ret, MOCK_OK);
	assert_int_equal(desc_size, MR_DESC_SIZE);
}

/* rpma_mr_get_descriptor() unit test */

/*
 * get_descriptor__mr_NULL - NULL mr is invalid
 */
static void
get_descriptor__mr_NULL(void **unused)
{
	/* run test */
	int ret = rpma_mr_get_descriptor(NULL, MOCK_DESC);

	/* verify the result */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * get_descriptor__desc_NULL - NULL desc is invalid
 */
static void
get_descriptor__desc_NULL(void **pprestate)
{
	struct prestate *prestate = *pprestate;
	struct rpma_mr_local *mr = prestate->mr;

	/* run test */
	int ret = rpma_mr_get_descriptor(mr, NULL);

	/* verify the result */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * get_descriptor__mr_NULL_desc_NULL - NULL mr and NULL desc are invalid
 */
static void
get_descriptor__mr_NULL_desc_NULL(void **unused)
{
	/* run test */
	int ret = rpma_mr_get_descriptor(NULL, NULL);

	/* verify the result */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/* rpma_mr_remote_from_descriptor() unit test */

/*
 * remote_from_descriptor__desc_NULL - NULL desc is invalid
 */
static void
remote_from_descriptor__desc_NULL(void **unused)
{
	/* run test */
	struct rpma_mr_remote *mr = NULL;
	int ret = rpma_mr_remote_from_descriptor(NULL, MR_DESC_SIZE, &mr);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
	assert_null(mr);
}

/*
 * remote_from_descriptor__mr_ptr_NULL - NULL mr_ptr is invalid
 */
static void
remote_from_descriptor__mr_ptr_NULL(void **unused)
{
	/* run test */
	int ret = rpma_mr_remote_from_descriptor(&Desc_exp_pmem,
			MR_DESC_SIZE, NULL);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * remote_from_descriptor__mr_ptr_NULL_desc_NULL - NULL mr_ptr and NULL
 * desc are invalid
 */
static void
remote_from_descriptor__mr_ptr_NULL_desc_NULL(void **unused)
{
	/* run test */
	int ret = rpma_mr_remote_from_descriptor(NULL, MR_DESC_SIZE, NULL);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * remote_from_descriptor__invalid_desc_size - invalid desc_size
 */
static void
remote_from_descriptor__invalid_desc_size(void **unused)
{
	/* run test */
	struct rpma_mr_remote *mr = NULL;
	int ret = rpma_mr_remote_from_descriptor(&Desc_exp_pmem,
			INVALID_MR_DESC_SIZE, &mr);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * remote_from_descriptor__malloc_ERRNO - malloc() fails with MOCK_ERRNO
 */
static void
remote_from_descriptor__malloc_ERRNO(void **unused)
{
	/* configure mock */
	will_return(__wrap__test_malloc, MOCK_ERRNO);

	/* run test */
	struct rpma_mr_remote *mr = NULL;
	int ret = rpma_mr_remote_from_descriptor(&Desc_exp_pmem,
			MR_DESC_SIZE, &mr);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_NOMEM);
}

/*
 * remote_from_descriptor__buff_usage_equal_zero - buff with invalid contents
 * should be detected as long as it breaks placement value
 */
static void
remote_from_descriptor__buff_usage_equal_zero(void **unused)
{
	char desc_invalid[MR_DESC_SIZE];
	memset(desc_invalid, 0xff, MR_DESC_SIZE - 1);

	/* set usage to 0 */
	desc_invalid[MR_DESC_SIZE - 1] = 0;

	/* configure mock */
	will_return_maybe(__wrap__test_malloc, MOCK_OK);

	/* run test */
	struct rpma_mr_remote *mr = NULL;
	int ret = rpma_mr_remote_from_descriptor(&desc_invalid,
			MR_DESC_SIZE, &mr);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
	assert_null(mr);
}

/* rpma_mr_remote_delete() unit test */

/*
 * remote_delete__mr_ptr_NULL - NULL mr_ptr is invalid
 */
static void
remote_delete__mr_ptr_NULL(void **unused)
{
	/* run test */
	int ret = rpma_mr_remote_delete(NULL);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * remote_delete__mr_NULL - NULL mr should exit quickly
 */
static void
remote_delete__mr_NULL(void **unused)
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
 * remote_get_size__mr_ptr_NULL - NULL mr_ptr is invalid
 */
static void
remote_get_size__mr_ptr_NULL(void **unused)
{
	/* run test */
	size_t size = 0;
	int ret = rpma_mr_remote_get_size(NULL, &size);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
	assert_int_equal(size, 0);
}

/*
 * remote_get_size__size_NULL - NULL size pointer is invalid
 */
static void
remote_get_size__size_NULL(void **mr_ptr)
{
	struct rpma_mr_remote *mr = *mr_ptr;

	/* run test */
	int ret = rpma_mr_remote_get_size(mr, NULL);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * remote_get_size__mr_ptr_NULL_size_NULL - NULL mr_ptr and NULL size
 * pointer are invalid
 */
static void
remote_get_size__mr_ptr_NULL_size_NULL(void **unused)
{
	/* run test */
	int ret = rpma_mr_remote_get_size(NULL, NULL);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * remote_get_size__success - rpma_mr_remote_get_size() success
 */
static void
remote_get_size__success(void **mr_ptr)
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
 * get_descriptor__desc_alignment - try rpma_mr_get_descriptor() with
 * a miscellaneous input descriptor alignment just to be sure the implementation
 * does not prefer certain alignments.
 */
static void
get_descriptor__desc_alignment(void **pprestate)
{
	struct prestate *prestate = *pprestate;
	struct rpma_mr_local *mr = prestate->mr;

	char buff_base[MR_DESC_SIZE * 2];
	char pattern[MR_DESC_SIZE * 2];
	memset(pattern, 0xff, MR_DESC_SIZE * 2);
	void *desc = NULL;
	int ret = 0;

	/*
	 * Generate a miscellaneous output descriptor alignment just to be sure
	 * the implementation does not prefer certain alignments.
	 */
	for (uintptr_t i = 0; i < MR_DESC_SIZE; ++i) {
		memset(buff_base, 0xff, MR_DESC_SIZE * 2);

		/* run test */
		desc = buff_base + i;
		ret = rpma_mr_get_descriptor(mr, desc);

		/* verify the results */
		assert_int_equal(ret, 0);
		assert_memory_equal(desc, &Desc_exp_pmem, MR_DESC_SIZE);
		assert_memory_equal(buff_base, pattern, i);
		assert_memory_equal(
				buff_base + i + MR_DESC_SIZE,
				pattern + i + MR_DESC_SIZE,
				MR_DESC_SIZE - i);
	}
}

/*
 * remote_from_descriptor__desc_alignment -- try
 * rpma_mr_remote_from_descriptor() with a miscellaneous output descriptor
 * alignment just to be sure the implementation does not prefer certain
 * alignments.
 */
static void
remote_from_descriptor__desc_alignment(void **unused)
{
	char buff_base[MR_DESC_SIZE * 2];
	char pattern[MR_DESC_SIZE * 2];
	memset(pattern, 0xff, MR_DESC_SIZE * 2);

	void *desc = NULL;
	struct rpma_mr_remote *mr = NULL;
	size_t size = 0;
	int usage = 0;
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
		desc = buff_base + i;
		const void *desc_src = (i % 2) ?
				&Desc_exp_pmem : &Desc_exp_dram;
		memcpy(desc, desc_src, MR_DESC_SIZE);

		/* specify the flush_type */
		int flush_type = (i % 2) ?
				RPMA_MR_USAGE_FLUSH_TYPE_PERSISTENT :
				RPMA_MR_USAGE_FLUSH_TYPE_VISIBILITY;

		/* run test */
		ret = rpma_mr_remote_from_descriptor(desc, MR_DESC_SIZE, &mr);

		/* verify the results */
		assert_int_equal(ret, MOCK_OK);
		assert_non_null(mr);
		ret = rpma_mr_remote_get_size(mr, &size);
		assert_int_equal(ret, MOCK_OK);
		assert_int_equal(size, MOCK_SIZE);
		ret = rpma_mr_remote_get_flush_type(mr, &usage);
		assert_int_equal(ret, MOCK_OK);
		assert_int_equal(usage, flush_type);
		/*
		 * It is not easy to verify whether the values
		 * addr and rkey are correct.
		 */

		/* cleanup */
		ret = rpma_mr_remote_delete(&mr);
		assert_int_equal(ret, MOCK_OK);
		assert_null(mr);
	}
}

static struct prestate prestate =
		{RPMA_MR_USAGE_READ_SRC | RPMA_MR_USAGE_FLUSH_TYPE_PERSISTENT,
		IBV_ACCESS_REMOTE_READ, NULL};

static const struct CMUnitTest tests_descriptor[] = {
	/* rpma_mr_get_descriptor_size() unit test */
	cmocka_unit_test(get_descriptor_size__mr_NULL),
	cmocka_unit_test_prestate_setup_teardown(
		get_descriptor_size__desc_size_NULL,
		setup__reg_success,
		teardown__dereg_success,
		&prestate),
	cmocka_unit_test(get_descriptor_size__mr_desc_size_NULL),
	cmocka_unit_test_prestate_setup_teardown(
		get_descriptor_size__success,
		setup__reg_success,
		teardown__dereg_success,
		&prestate),

	/* rpma_mr_get_descriptor() unit test */
	cmocka_unit_test(get_descriptor__mr_NULL),
	cmocka_unit_test_prestate_setup_teardown(
		get_descriptor__desc_NULL,
		setup__reg_success,
		teardown__dereg_success,
		&prestate),
	cmocka_unit_test(get_descriptor__mr_NULL_desc_NULL),

	/* rpma_mr_remote_from_descriptor() unit test */
	cmocka_unit_test(remote_from_descriptor__desc_NULL),
	cmocka_unit_test(remote_from_descriptor__mr_ptr_NULL),
	cmocka_unit_test(
		remote_from_descriptor__mr_ptr_NULL_desc_NULL),
	cmocka_unit_test(remote_from_descriptor__invalid_desc_size),
	cmocka_unit_test(remote_from_descriptor__malloc_ERRNO),
	cmocka_unit_test(remote_from_descriptor__buff_usage_equal_zero),

	/* rpma_mr_remote_delete() unit test */
	cmocka_unit_test(remote_delete__mr_ptr_NULL),
	cmocka_unit_test(remote_delete__mr_NULL),

	/* rpma_mr_remote_get_size() unit test */
	cmocka_unit_test(remote_get_size__mr_ptr_NULL),
	cmocka_unit_test_setup_teardown(remote_get_size__size_NULL,
		setup__mr_remote, teardown__mr_remote),
	cmocka_unit_test(remote_get_size__mr_ptr_NULL_size_NULL),
	cmocka_unit_test_setup_teardown(remote_get_size__success,
		setup__mr_remote, teardown__mr_remote),

	/*
	 * rpma_mr_get_descriptor()/rpma_mr_remote_from_descriptor()
	 * buffer alignment
	 */
	cmocka_unit_test_prestate_setup_teardown(
		get_descriptor__desc_alignment,
		setup__reg_success, teardown__dereg_success,
		&prestate),
	cmocka_unit_test(remote_from_descriptor__desc_alignment),
	cmocka_unit_test(NULL)
};

int
main(int argc, char *argv[])
{
	return cmocka_run_group_tests(tests_descriptor, NULL, NULL);
}
