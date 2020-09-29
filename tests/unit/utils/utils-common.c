// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020, Intel Corporation */

/*
 * utils-common.c -- the utils unit tests common functions
 */

#include <stdio.h>
#include <stdlib.h>
#include <rdma/rdma_cma.h>

#include "cmocka_headers.h"
#include "librpma.h"
#include "mocks-ibverbs.h"
#include "info.h"
#include "test-common.h"

/*
 * rpma_info_new -- mock of rpma_info_new
 */
int
rpma_info_new(const char *addr, const char *port, enum rpma_info_side side,
		struct rpma_info **info_ptr)
{
	/*
	 * rpma_info_new() and rdma_create_id() may be called in any order.
	 * If the first one fails, then the second one won't be called,
	 * so we cannot add cmocka's expects here.
	 * Otherwise, unconsumed expects would cause a test failure.
	 */
	assert_string_equal(addr, MOCK_IP_ADDRESS);
	assert_null(port);
	assert_true(side == RPMA_INFO_PASSIVE || side == RPMA_INFO_ACTIVE);

	*info_ptr = mock_type(struct rpma_info *);
	if (*info_ptr == NULL) {
		int result = mock_type(int);
		/* XXX validate the errno handling */
		if (result == RPMA_E_PROVIDER)
			errno = mock_type(int);

		return result;
	}

	expect_value(rpma_info_delete, *info_ptr, *info_ptr);

	return 0;
}

/*
 * rdma_create_id -- mock of rdma_create_id
 */
int
rdma_create_id(struct rdma_event_channel *channel,
		struct rdma_cm_id **id, void *context,
		enum rdma_port_space ps)
{
	/*
	 * rpma_info_new() and rdma_create_id() may be called in any order.
	 * If the first one fails, then the second one won't be called,
	 * so we cannot add cmocka's expects here.
	 * Otherwise, unconsumed expects would cause a test failure.
	 */
	assert_non_null(id);
	assert_null(context);
	assert_int_equal(ps, RDMA_PS_TCP);

	/* allocate (struct rdma_cm_id *) */
	*id = mock_type(struct rdma_cm_id *);
	if (*id == NULL) {
		errno = mock_type(int);
		return -1;
	}

	expect_value(rdma_destroy_id, id, *id);

	return 0;
}

/*
 * rpma_info_bind_addr -- mock of rpma_info_bind_addr
 */
int
rpma_info_bind_addr(const struct rpma_info *info, struct rdma_cm_id *id)
{
	check_expected(info);
	check_expected(id);

	/* XXX validate the errno handling */
	int ret = mock_type(int);
	if (ret)
		errno = mock_type(int);

	return ret;
}

/*
 * rpma_info_resolve_addr -- mock of rpma_info_resolve_addr
 */
int
rpma_info_resolve_addr(const struct rpma_info *info, struct rdma_cm_id *id,
		int timeout_ms)
{
	check_expected(info);
	check_expected(id);
	assert_int_equal(timeout_ms, RPMA_DEFAULT_TIMEOUT_MS);

	/* XXX validate the errno handling */
	int ret = mock_type(int);
	if (ret)
		errno = mock_type(int);

	return ret;
}

/*
 * rdma_destroy_id -- mock of rdma_destroy_id
 */
int
rdma_destroy_id(struct rdma_cm_id *id)
{
	check_expected(id);

	errno = mock_type(int);
	if (errno)
		return -1;

	return 0;
}

/*
 * rpma_info_delete -- mock of rpma_info_delete
 */
int
rpma_info_delete(struct rpma_info **info_ptr)
{
	if (info_ptr == NULL)
		return RPMA_E_INVAL;

	check_expected(*info_ptr);

	return 0;
}
