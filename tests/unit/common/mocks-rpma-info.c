// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020, Intel Corporation */

/*
 * mocks-rpma-info.c -- librpma info.c module mocks
 */

#include <rdma/rdma_cma.h>
#include <librpma.h>

#include "cmocka_headers.h"
#include "info.h"
#include "test-common.h"

/*
 * rpma_info_new -- mock of rpma_info_new
 */
int
rpma_info_new(const char *addr, const char *port, enum rpma_info_side side,
		struct rpma_info **info_ptr)
{
	assert_string_equal(addr, MOCK_IP_ADDRESS);
	assert_string_equal(port, MOCK_PORT);
	assert_int_equal(side, RPMA_INFO_ACTIVE);

	*info_ptr = mock_type(struct rpma_info *);
	if (*info_ptr == NULL) {
		int result = mock_type(int);
		assert_int_not_equal(result, 0);

		/* XXX validate the errno handling */
		if (result == RPMA_E_PROVIDER)
			errno = mock_type(int);

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

	/* if argument is correct it cannot fail */
	return 0;
}

/*
 * rpma_info_resolve_addr -- mock of rpma_info_resolve_addr
 */
int
rpma_info_resolve_addr(const struct rpma_info *info, struct rdma_cm_id *id,
		int timeout_ms)
{
	assert_int_equal(info, MOCK_INFO);
	check_expected(id);
	check_expected(timeout_ms);

	int ret = mock_type(int);
	/* XXX validate the errno handling */
	if (ret == RPMA_E_PROVIDER)
		errno = mock_type(int);

	if (ret == MOCK_OK)
		expect_value(rdma_resolve_route, id, id);

	return ret;
}

/*
 * rpma_info_bind_addr -- mock of rpma_info_bind_addr
 */
int
rpma_info_bind_addr(const struct rpma_info *info, struct rdma_cm_id *id)
{
	check_expected(info);
	check_expected(id);

	int ret = mock_type(int);
	/* XXX validate the errno handling */
	if (ret)
		errno = mock_type(int);

	return ret;
}
