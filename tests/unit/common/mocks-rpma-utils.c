// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020, Intel Corporation */

/*
 * mocks-rpma-utils.c -- librpma rpma.c module mocks (rpma_utils_*)
 */

#include <rdma/rdma_cma.h>
#include <librpma.h>

#include "cmocka_headers.h"
#include "mocks-ibverbs.h"
#include "rpma_err.h"
#include "test-common.h"

/*
 * rpma_utils_ibv_context_is_odp_capable --
 * rpma_utils_ibv_context_is_odp_capable() mock
 */
int
rpma_utils_ibv_context_is_odp_capable(struct ibv_context *dev,
		int *is_odp_capable)
{
	assert_ptr_equal(dev, MOCK_VERBS);
	assert_non_null(is_odp_capable);

	*is_odp_capable = mock_type(int);
	if (*is_odp_capable == MOCK_ERR_PENDING) {
		int ret = mock_type(int);
		if (ret == RPMA_E_PROVIDER)
			Rpma_provider_error = mock_type(int);
		return ret;
	}

	return 0;
}
