/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2020, Intel Corporation
 */

/*
 * mr-test.c -- the memory region unit tests
 */

#include <stdlib.h>

#include "cmocka_headers.h"
#include "mr.h"
#include "librpma.h"
#include "rpma_err.h"

#include <infiniband/verbs.h>

/*
 * Calculate an exptected value of required serialized memory region:
 * exp = raddr + size + rkey + plt
 *
 * Note: plt has two possible values so it fits perfectly into a single
 * uint8_t value.
 */
#define SERIALIZED_SIZE_EXP \
	(sizeof(uint64_t) * 2 + sizeof(uint32_t) + sizeof(uint8_t))

#define MOCK_PEER (struct rpma_peer *)0xFEEF
#define MOCK_OK	0
#define MR_SIZE 867 /* a random value */

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

/* setups & teardowns */

/*
 * setup_mr_local -- register a local memory region's structure
 */
int
setup_mr_local(void **mr_ptr)
{
	static char ptr[MR_SIZE] = {0};

	/* prepare local memory region structure */
	struct rpma_mr_local *mr = NULL;
	int ret = rpma_mr_reg(MOCK_PEER, (void *)ptr, MR_SIZE,
			RPMA_MR_USAGE_READ_SRC, RPMA_MR_PLT_VOLATILE, &mr);

	/* verify the results */
	assert_int_equal(ret, MOCK_OK);
	assert_non_null(mr);

	*mr_ptr = mr;

	return 0;
}

/*
 * teardown_mr_local -- deregister the local memory region's structure
 */
int
teardown_mr_local(void **mr_ptr)
{
	struct rpma_mr_local *mr = *mr_ptr;

	/* deregister the local memory region structure */
	int ret = rpma_mr_dereg(&mr);

	/* verify the results */
	assert_int_equal(ret, MOCK_OK);
	assert_null(mr);

	*mr_ptr = NULL;

	return 0;
}

/*
 * all the resources used between setup_mr_local_and_buff() and
 * teardown_mr_local_and_buff()
 */
struct mr_local_and_buff {
	struct rpma_mr_local *local;
	/*
	 * the size is one byte bigger for the sake of the size mismatch unit
	 * tests
	 */
	char buff[SERIALIZED_SIZE_EXP + 1];
	char buff_copy[SERIALIZED_SIZE_EXP];
};

/*
 * setup_mr_local_and_buff -- register a local memory region's structure along
 * with a buffer with its serialized form in two copies
 */
int
setup_mr_local_and_buff(void **mrb_ptr)
{
	static struct mr_local_and_buff mrb = {0};
	static char ptr[MR_SIZE] = {0};

	/* prepare local memory region structure */
	struct rpma_mr_local *mr = NULL;
	int ret = rpma_mr_reg(MOCK_PEER, (void *)ptr, MR_SIZE,
			RPMA_MR_USAGE_READ_SRC, RPMA_MR_PLT_VOLATILE, &mr);

	/* verify the results */
	assert_int_equal(ret, MOCK_OK);
	assert_non_null(mr);

	/* serialize the local memory region */
	ret = rpma_mr_serialize(mrb.local, mrb.buff);
	assert_int_equal(ret, MOCK_OK);

	/* prepare a buff copy for later validation */
	memcpy(mrb.buff_copy, mrb.buff, SERIALIZED_SIZE_EXP);

	*mrb_ptr = &mrb;

	return 0;
}

/*
 * teardown_mr_local_and_buff -- deregister the local memory region's structure
 */
int
teardown_mr_local_and_buff(void **mrb_ptr)
{
	struct mr_local_and_buff *mrb = *mrb_ptr;

	/* verify the buff contents is not modified */
	assert_memory_equal(mrb->buff, mrb->buff_copy, SERIALIZED_SIZE_EXP);

	/* deregister the local memory region structure */
	int ret = rpma_mr_dereg(&mrb->local);

	/* verify the results */
	assert_int_equal(ret, MOCK_OK);
	assert_null(mrb->local);

	*mrb_ptr = NULL;

	return 0;
}

/*
 * all the resources used between setup_mr_local_and_remote() and
 * teardown_mr_local_and_remote()
 */
struct mr_local_and_remote {
	struct rpma_mr_local *local;
	struct rpma_mr_remote *remote;
};

/*
 * setup_mr_local_and_remote -- register a local memory region's structure along
 * with remote memory region for the same region
 */
int
setup_mr_local_and_remote(void **mrs_ptr)
{
	static struct mr_local_and_remote mrs = {0};
	char ptr[MR_SIZE] = {0};

	/* prepare local memory region structure */
	int ret = rpma_mr_reg(MOCK_PEER, (void *)ptr, MR_SIZE,
			RPMA_MR_USAGE_READ_SRC, RPMA_MR_PLT_VOLATILE,
			&mrs.local);

	/* verify the results */
	assert_int_equal(ret, MOCK_OK);
	assert_non_null(mrs.local);

	/* serialize the local memory region */
	char buff[SERIALIZED_SIZE_EXP] = {0};
	char buff_copy[SERIALIZED_SIZE_EXP] = {0};
	ret = rpma_mr_serialize(mrs.local, buff);
	assert_int_equal(ret, MOCK_OK);

	/* prepare a buff copy for later validation */
	memcpy(buff_copy, buff, SERIALIZED_SIZE_EXP);

	/* deserialize the memory region as a remote access structure */
	ret = rpma_mr_deserialize(buff, SERIALIZED_SIZE_EXP, &mrs.remote);

	/* verify the results */
	assert_int_equal(ret, MOCK_OK);
	assert_non_null(mrs.remote);
	assert_memory_equal(buff, buff_copy, SERIALIZED_SIZE_EXP);

	*mrs_ptr = &mrs;

	return 0;
}

/*
 * teardown_mr_local_and_remote -- deregister the local and remote memory
 * region's structure
 */
int
teardown_mr_local_and_remote(void **mrs_ptr)
{
	struct mr_local_and_remote *mrs = *mrs_ptr;

	/* delete the remote memory region's structure */
	int ret = rpma_mr_remote_delete(&mrs->remote);
	assert_int_equal(ret, MOCK_OK);
	assert_null(mrs->remote);

	/* deregister the local memory region's structure */
	ret = rpma_mr_dereg(&mrs->local);
	assert_int_equal(ret, MOCK_OK);
	assert_null(mrs->local);

	*mrs_ptr = NULL;

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
	assert_int_equal(ret, RPMA_E_INVAL);
}

/* rpma_mr_deserialize() unit test */

/*
 * test_deserialize__buff_NULL - NULL buff is invalid
 */
static void
test_deserialize__buff_NULL(void **mr_ptr)
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
test_deserialize__mr_ptr_NULL(void **mrb_ptr)
{
	struct mr_local_and_buff *mrb = *mrb_ptr;

	/* run test */
	int ret = rpma_mr_deserialize(mrb->buff, SERIALIZED_SIZE_EXP, NULL);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * test_deserialize__mr_ptr_NULL_buff_NULL - NULL mr_ptr and NULL buff are
 * invalid
 */
static void
test_deserialize__mr_ptr_NULL_buff_NULL(void **mr_ptr)
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
test_deserialize__too_big_size(void **mrb_ptr)
{
	struct mr_local_and_buff *mrb = *mrb_ptr;

	/* run test */
	int ret = rpma_mr_deserialize(mrb->buff, SERIALIZED_SIZE_EXP + 1, NULL);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_NOSUPP);
}

/*
 * test_deserialize__too_small_size - SERIALIZED_SIZE_EXP - 1 is invalid
 */
static void
test_deserialize__too_small_size(void **mrb_ptr)
{
	struct mr_local_and_buff *mrb = *mrb_ptr;

	/* run test */
	int ret = rpma_mr_deserialize(mrb->buff, SERIALIZED_SIZE_EXP - 1, NULL);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_NOSUPP);
}

/*
 * test_deserialize__malloc_ENOMEM - malloc() fail with ENOMEM
 */
static void
test_deserialize__malloc_ENOMEM(void **mrb_ptr)
{
	struct mr_local_and_buff *mrb = *mrb_ptr;

	/* configure mock */
	will_return_maybe(__wrap__test_malloc, ENOMEM);

	/* run test */
	struct rpma_mr_remote *mr = NULL;
	int ret = rpma_mr_deserialize(mrb->buff, SERIALIZED_SIZE_EXP, &mr);

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
test_remote_get_size__size_NULL(void **mrs_ptr)
{
	struct mr_local_and_remote *mrs = *mrs_ptr;

	/* run test */
	int ret = rpma_mr_remote_get_size(mrs->remote, NULL);

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

/* rpma_mr_serialize()/_deserialize()/_remote_delete() lifecycle */

/*
 * test_lifecycle - use all of the APIs in sequence:
 * - rpma_mr_reg()
 * - rpma_mr_serialize()
 * - rpma_mr_deserialize()
 * - rpma_mr_remote_get_size()
 * - rpma_mr_remote_delete()
 * - rpma_mr_dereg()
 */
static void
test_lifecycle(void **mrs_ptr)
{
	/*
	 * The thing is done by setup_mr_local_and_remote() and
	 * teardown_mr_local_and_remote().
	 */

	struct mr_local_and_remote *mrs = *mrs_ptr;

	/* verify the remote memory region correctness */
	size_t size = 0;
	int ret = rpma_mr_remote_get_size(mrs->remote, &size);
	assert_int_equal(ret, MOCK_OK);
	assert_int_equal(size, MR_SIZE);
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
				setup_mr_local, teardown_mr_local),
		cmocka_unit_test(test_serialize__mr_NULL_buff_NULL),

		/* rpma_mr_deserialize() unit test */
		cmocka_unit_test(test_deserialize__buff_NULL),
		cmocka_unit_test_setup_teardown(test_deserialize__mr_ptr_NULL,
				setup_mr_local_and_buff,
				teardown_mr_local_and_buff),
		cmocka_unit_test(test_deserialize__mr_ptr_NULL_buff_NULL),
		cmocka_unit_test_setup_teardown(test_deserialize__too_big_size,
				setup_mr_local_and_buff,
				teardown_mr_local_and_buff),
		cmocka_unit_test_setup_teardown(
				test_deserialize__too_small_size,
				setup_mr_local_and_buff,
				teardown_mr_local_and_buff),
		cmocka_unit_test_setup_teardown(test_deserialize__malloc_ENOMEM,
				setup_mr_local_and_buff,
				teardown_mr_local_and_buff),
		cmocka_unit_test(test_deserialize__buff_plt_invalid),

		/* rpma_mr_remote_get_size() unit test */
		cmocka_unit_test(test_remote_get_size__mr_ptr_NULL),
		cmocka_unit_test_setup_teardown(test_remote_get_size__size_NULL,
				setup_mr_local_and_remote,
				teardown_mr_local_and_remote),
		cmocka_unit_test(test_remote_get_size__mr_ptr_NULL_size_NULL),

		/* rpma_mr_remote_delete() unit test */
		cmocka_unit_test(test_remote_delete__mr_ptr_NULL),
		cmocka_unit_test(test_remote_delete__mr_NULL),

		/*
		 * rpma_mr_serialize()/_deserialize()/_remote_delete()
		 * lifecycle
		 */
		cmocka_unit_test_setup_teardown(test_lifecycle,
				setup_mr_local_and_remote,
				teardown_mr_local_and_remote),
	};

	return cmocka_run_group_tests(tests, NULL, NULL);
}
