/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2020, Intel Corporation
 */

/*
 * mocks-rpma-flush.c -- librpma flush.c module mocks
 */

#include <rdma/rdma_cma.h>
#include <librpma.h>

#include "rpma_err.h"
#include "cmocka_headers.h"
#include "mocks-rpma-flush.h"
#include "test-common.h"

struct rpma_flush Rpma_flush;

/*
 * rpma_flush_new -- rpma_flush_new() mock
 */
int
rpma_flush_new(struct rpma_peer *peer, struct rpma_flush **flush_ptr)
{
	assert_non_null(flush_ptr);

	int ret = mock_type(int);
	if (ret == MOCK_OK)
		*flush_ptr = MOCK_FLUSH;

	return ret;
}

/*
 * rpma_flush_delete -- rpma_flush_delete() mock
 */
int
rpma_flush_delete(struct rpma_flush **flush_ptr)
{
	assert_ptr_equal(*flush_ptr, MOCK_FLUSH);
	*flush_ptr = NULL;

	return 0;
}
