// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020, Intel Corporation */

/*
 * mocks-rpma-peer_cfg.c -- librpma peer_cfg.c module mocks
 */

#include <librpma.h>

#include "cmocka_headers.h"
#include "mocks-rpma-peer_cfg.h"
#include "test-common.h"

/*
 * rpma_peer_cfg_get_direct_write_to_pmem -- mock of the original one
 */
int
rpma_peer_cfg_get_direct_write_to_pmem(struct rpma_peer_cfg *pcfg,
		bool *supported)
{
	assert_non_null(pcfg);
	assert_non_null(supported);

	return 0;
}
