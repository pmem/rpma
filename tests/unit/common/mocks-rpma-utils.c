// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020, Intel Corporation */

/*
 * mocks-rpma-utils.c -- librpma utils.c module mocks (rpma_utils_*)
 */

#include <rdma/rdma_cma.h>
#include <librpma.h>

#include "cmocka_headers.h"
#include "mocks-ibverbs.h"
#include "test-common.h"

/*
 * rpma_utils_ibv_context_is_odp_capable --
 * rpma_utils_ibv_context_is_odp_capable() mock
 */
int
rpma_utils_ibv_context_is_odp_capable(struct ibv_context *ibv_ctx,
		int *is_odp_capable)
{
	assert_ptr_equal(ibv_ctx, MOCK_VERBS);
	assert_non_null(is_odp_capable);

	*is_odp_capable = mock_type(int);
	if (*is_odp_capable == MOCK_ERR_PENDING) {
		int ret = mock_type(int);
		/* XXX validate the errno handling */
		if (ret == RPMA_E_PROVIDER)
			errno = mock_type(int);
		return ret;
	}

	return 0;
}

/*
 * rpma_utils_conn_event_2str -- rpma_utils_conn_event_2str() mock
 */
const char *
rpma_utils_conn_event_2str(enum rpma_conn_event conn_event)
{
	return "";
}
