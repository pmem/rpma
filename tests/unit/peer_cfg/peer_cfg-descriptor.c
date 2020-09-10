// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020, Intel Corporation */

/*
 * peer_cfg-direct_write_to_pmem.c -- the peer_cfg descriptor unit tests
 *
 * APIs covered:
 * - rpma_peer_cfg_get_descriptor()
 * - rpma_peer_cfg_from_descriptor()
 */

#include "peer_cfg-common.h"
#include "test-common.h"

/*
 * get_desc__pcfg_NULL -- NULL pcfg is invalid
 */
static void
get_desc__pcfg_NULL(void **unused)
{
	/* run test */
	rpma_peer_cfg_descriptor desc;
	int ret = rpma_peer_cfg_get_descriptor(NULL, &desc);

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
	int ret = rpma_peer_cfg_from_descriptor(NULL, MOCK_PEER_PCFG_PTR);

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
	rpma_peer_cfg_descriptor desc;
	int ret = rpma_peer_cfg_from_descriptor(&desc, NULL);

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
	int ret = rpma_peer_cfg_from_descriptor(NULL, NULL);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * from_desc__malloc_ENOMEM -- malloc() fails with ENOMEM
 */
static void
from_desc__malloc_ENOMEM(void **unused)
{
	/* configure mocks */
	will_return(__wrap__test_malloc, ENOMEM);

	/* run test of rpma_peer_cfg_from_descriptor() */
	rpma_peer_cfg_descriptor desc;
	struct rpma_peer_cfg *pcfg = NULL;
	int ret = rpma_peer_cfg_from_descriptor(&desc, &pcfg);

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
	assert_int_equal(RPMA_PEER_CFG_DESCRIPTOR_SIZE, 1);

	for (uint8_t supported = 0; supported < 2; supported++) {
		/* configure mocks */
		will_return(__wrap__test_malloc, MOCK_OK);

		/* run test of rpma_peer_cfg_from_descriptor() */
		rpma_peer_cfg_descriptor desc;
		desc.data[0] = (uint8_t)supported;
		struct rpma_peer_cfg *pcfg;
		int ret = rpma_peer_cfg_from_descriptor(&desc, &pcfg);

		/* verify the results */
		assert_int_equal(ret, MOCK_OK);
		rpma_peer_cfg_descriptor desc2;
		ret = rpma_peer_cfg_get_descriptor(pcfg, &desc2);
		assert_int_equal(ret, MOCK_OK);
		assert_int_equal(desc2.data[0], supported);

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
	assert_int_equal(RPMA_PEER_CFG_DESCRIPTOR_SIZE, 1);

	/* run test of rpma_peer_cfg_get_descriptor() */
	rpma_peer_cfg_descriptor desc;
	int ret = rpma_peer_cfg_get_descriptor(cstate->cfg,
			&desc);

	/* verify the results */
	assert_int_equal(ret, MOCK_OK);
	assert_int_equal(desc.data[0], (uint8_t)false);

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
	assert_int_equal(desc.data[0], (uint8_t)true);
}


static const struct CMUnitTest test_direct_write_to_pmem[] = {
	/* rpma_peer_cfg_get_descriptor() unit tests */
	cmocka_unit_test(get_desc__pcfg_NULL),
	cmocka_unit_test(get_desc__desc_NULL),
	cmocka_unit_test(get_desc__pcfg_desc_NULL),

	/* rpma_peer_cfg_from_descriptor() unit tests */
	cmocka_unit_test(from_desc__desc_NULL),
	cmocka_unit_test(from_desc__pcfg_ptr_NULL),
	cmocka_unit_test(from_desc__pcfg_ptr_desc_NULL),
	cmocka_unit_test(from_desc__malloc_ENOMEM),
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
