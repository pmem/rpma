/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2020, Intel Corporation
 */

/*
 * mocks-info.c -- an info.c module mocks
 */

#include <rdma/rdma_cma.h>
#include <librpma.h>

#include "cmocka_headers.h"
#include "info.h"
#include "rpma_err.h"
#include "test-common.h"

/*
 * rpma_info_new -- mock of rpma_info_new
 */
int
rpma_info_new(const char *addr, const char *service, enum rpma_info_side side,
		struct rpma_info **info_ptr)
{
	assert_string_equal(addr, MOCK_IP_ADDRESS);
	assert_string_equal(service, MOCK_SERVICE);
	assert_int_equal(side, RPMA_INFO_ACTIVE);

	*info_ptr = mock_type(struct rpma_info *);
	if (*info_ptr == NULL) {
		int result = mock_type(int);
		assert_int_not_equal(result, 0);

		if (result == RPMA_E_PROVIDER)
			Rpma_provider_error = mock_type(int);

		return result;
	}

	return 0;
}

/*
 * rpma_info_delete -- mock of rpma_info_delete
 */
int
rpma_info_delete(struct rpma_info **info_ptr)
{
	assert_non_null(info_ptr);
	assert_int_equal(*info_ptr, MOCK_INFO);

	/* if argument is correct it connot fail */
	return 0;
}

/*
 * rpma_info_resolve_addr -- mock of rpma_info_resolve_addr
 */
int
rpma_info_resolve_addr(const struct rpma_info *info, struct rdma_cm_id *id)
{
	assert_int_equal(info, MOCK_INFO);
	check_expected(id);

	int ret = mock_type(int);
	if (ret == RPMA_E_PROVIDER)
		Rpma_provider_error = mock_type(int);

	if (ret == MOCK_OK)
		expect_value(rdma_resolve_route, id, id);

	return ret;
}
