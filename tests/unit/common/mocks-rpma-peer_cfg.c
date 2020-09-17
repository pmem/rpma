// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020, Intel Corporation */

/*
 * mocks-rpma-peer_cfg.c -- librpma peer_cfg.c module mocks
 */

#include <librpma.h>

#include "cmocka_headers.h"
#include "test-common.h"

/*
 * rpma_peer_cfg_get_direct_write_to_pmem -- mock of the original one
 */
int
rpma_peer_cfg_get_direct_write_to_pmem(struct rpma_peer_cfg *pcfg,
		bool *supported)
{
	assert_ptr_equal(pcfg, MOCK_PEER_PCFG);
	assert_non_null(supported);

	*supported = mock_type(bool);

	return 0;
}
