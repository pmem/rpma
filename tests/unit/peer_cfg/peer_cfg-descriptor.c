// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020, Intel Corporation */
/* Copyright 2021, Fujitsu */

/*
 * peer_cfg-direct_write_to_pmem.c -- the peer_cfg descriptor unit tests
 *
 * APIs covered:
 * - rpma_peer_cfg_get_descriptor_size()
 * - rpma_peer_cfg_get_descriptor()
 * - rpma_peer_cfg_from_descriptor()
 */

#include "peer_cfg-common.h"
#include "test-common.h"

/*
 * get_desc_size__pcfg_NULL -- NULL pcfg is invalid
 */
static void
get_desc_size__pcfg_NULL(void **unused)
{
	/* run test */
	size_t desc_size;
	int ret = rpma_peer_cfg_get_descriptor_size(NULL, &desc_size);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * get_desc_size__desc_size_NULL -- NULL desc_size is invalid
 */
static void
get_desc_size__desc_size_NULL(void **unused)
{
	/* run test */
	int ret = rpma_peer_cfg_get_descriptor_size(MOCK_PEER_PCFG, NULL);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * get_desc_size__pcfg_desc_size_NULL -- NULL pcfg and desc_size are invalid
 */
static void
get_desc_size__pcfg_desc_size_NULL(void **unused)
{
	/* run test */
	int ret = rpma_peer_cfg_get_descriptor_size(NULL, NULL);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * get_desc_size__success -- happy day scenario
 */
static void
get_desc_size__success(void **unused)
{
	/* run test */
	size_t desc_size;
	int ret = rpma_peer_cfg_get_descriptor_size(MOCK_PEER_PCFG, &desc_size);

	/* verify the results */
	assert_int_equal(ret, MOCK_OK);
	assert_int_equal(desc_size, MOCK_DESC_SIZE);
}

/*
 * get_desc__pcfg_NULL -- NULL pcfg is invalid
 */
static void
get_desc__pcfg_NULL(void **unused)
{
	/* run test */
	int ret = rpma_peer_cfg_get_descriptor(NULL, MOCK_DESC);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * get_desc__desc_NULL -- NULL desc is invalid
 */
static void
get_desc__desc_NULL(void **unused)
{
	/* run test */
	int ret = rpma_peer_cfg_get_descriptor(MOCK_PEER_PCFG, NULL);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * get_desc__pcfg_desc_NULL -- NULL pcfg and desc are invalid
 */
static void
get_desc__pcfg_desc_NULL(void **unused)
{
	/* run test */
	int ret = rpma_peer_cfg_get_descriptor(NULL, NULL);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * from_desc__desc_NULL -- NULL desc is invalid
 */
static void
from_desc__desc_NULL(void **unused)
{
	/* run test */
	int ret = rpma_peer_cfg_from_descriptor(NULL, MOCK_DESC_SIZE,
			MOCK_PEER_PCFG_PTR);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * from_desc__pcfg_ptr_NULL -- NULL pcfg_ptr is invalid
 */
static void
from_desc__pcfg_ptr_NULL(void **unused)
{
	/* run test */
	int ret = rpma_peer_cfg_from_descriptor(MOCK_DESC, MOCK_DESC_SIZE,
			NULL);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * from_desc__pcfg_ptr_desc_NULL -- NULL pcfg_ptr and desc are invalid
 */
static void
from_desc__pcfg_ptr_desc_NULL(void **unused)
{
	/* run test */
	int ret = rpma_peer_cfg_from_descriptor(NULL, MOCK_DESC_SIZE, NULL);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * from_desc__incorrect_desc_size -- incorrect size of the descriptor
 */
static void
from_desc__incorrect_desc_size(void **unused)
{
	/* configure mocks */
	will_return_maybe(__wrap__test_malloc, MOCK_ERRNO);

	/* run test of rpma_peer_cfg_from_descriptor() */
	struct rpma_peer_cfg *pcfg = NULL;
	int ret = rpma_peer_cfg_from_descriptor(MOCK_DESC, MOCK_WRONG_DESC_SIZE,
			&pcfg);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
	assert_null(pcfg);
}

/*
 * from_desc__malloc_ERRNO -- malloc() fails with MOCK_ERRNO
 */
static void
from_desc__malloc_ERRNO(void **unused)
{
	/* configure mocks */
	will_return(__wrap__test_malloc, MOCK_ERRNO);

	/* run test of rpma_peer_cfg_from_descriptor() */
	struct rpma_peer_cfg *pcfg = NULL;
	int ret = rpma_peer_cfg_from_descriptor(MOCK_DESC, MOCK_DESC_SIZE,
			&pcfg);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_NOMEM);
	assert_null(pcfg);
}

/*
 * from_desc__success -- happy day scenario
 */
static void
from_desc__success(void **unused)
{
	/* verify test conditions */
	size_t desc_size;
	(void) rpma_peer_cfg_get_descriptor_size(MOCK_PEER_PCFG, &desc_size);
	assert_int_equal(desc_size, MOCK_DESC_SIZE);

	for (uint8_t supp = 0; supp < 2; supp++) {
		/* configure mocks */
		will_return(__wrap__test_malloc, MOCK_OK);

		/* run test of rpma_peer_cfg_from_descriptor() */
		uint8_t desc[MOCK_DESC_SIZE];
		desc[0] = supp;
		struct rpma_peer_cfg *pcfg;
		int ret = rpma_peer_cfg_from_descriptor(desc, MOCK_DESC_SIZE,
				&pcfg);

		/* verify the results */
		assert_int_equal(ret, MOCK_OK);
		bool supported;
		ret = rpma_peer_cfg_get_direct_write_to_pmem(pcfg, &supported);
		assert_int_equal(ret, MOCK_OK);
		assert_int_equal(supported, supp);

		ret = rpma_peer_cfg_delete(&pcfg);
		assert_int_equal(ret, MOCK_OK);
	}
}

/*
 * get_desc__lifecycle -- happy day scenario
 */
static void
get_desc__lifecycle(void **cstate_ptr)
{
	struct peer_cfg_test_state *cstate = *cstate_ptr;

	/* verify test conditions */
	size_t desc_size;
	(void) rpma_peer_cfg_get_descriptor_size(MOCK_PEER_PCFG, &desc_size);
	assert_int_equal(desc_size, MOCK_DESC_SIZE);

	/* run test of rpma_peer_cfg_get_descriptor() */
	uint8_t desc[MOCK_DESC_SIZE];
	int ret = rpma_peer_cfg_get_descriptor(cstate->cfg, desc);

	/* verify the results */
	assert_int_equal(ret, MOCK_OK);
	assert_int_equal(desc[0], (uint8_t)false);

	/* run test of rpma_peer_cfg_set_direct_write_to_pmem() */
	ret = rpma_peer_cfg_set_direct_write_to_pmem(cstate->cfg,
			true);

	/* verify the results */
	assert_int_equal(ret, MOCK_OK);

	/* run test of rpma_peer_cfg_get_descriptor() */
	ret = rpma_peer_cfg_get_descriptor(cstate->cfg,
			&desc);

	/* verify the results */
	assert_int_equal(ret, MOCK_OK);
	assert_int_equal(desc[0], (uint8_t)true);
}


static const struct CMUnitTest test_direct_write_to_pmem[] = {
	/* rpma_peer_cfg_get_descriptor_size() unit tests */
	cmocka_unit_test(get_desc_size__pcfg_NULL),
	cmocka_unit_test(get_desc_size__desc_size_NULL),
	cmocka_unit_test(get_desc_size__pcfg_desc_size_NULL),
	cmocka_unit_test(get_desc_size__success),

	/* rpma_peer_cfg_get_descriptor() unit tests */
	cmocka_unit_test(get_desc__pcfg_NULL),
	cmocka_unit_test(get_desc__desc_NULL),
	cmocka_unit_test(get_desc__pcfg_desc_NULL),

	/* rpma_peer_cfg_from_descriptor() unit tests */
	cmocka_unit_test(from_desc__desc_NULL),
	cmocka_unit_test(from_desc__pcfg_ptr_NULL),
	cmocka_unit_test(from_desc__pcfg_ptr_desc_NULL),
	cmocka_unit_test(from_desc__incorrect_desc_size),
	cmocka_unit_test(from_desc__malloc_ERRNO),
	cmocka_unit_test(from_desc__success),

	/* rpma_peer_cfg_get_descriptor() lifecycle */
	cmocka_unit_test_setup_teardown(get_desc__lifecycle,
		setup__peer_cfg, teardown__peer_cfg),
};

int
main(int argc, char *argv[])
{
	return cmocka_run_group_tests(test_direct_write_to_pmem, NULL, NULL);
}
