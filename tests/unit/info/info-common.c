// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020, Intel Corporation */

/*
 * info-common.c -- common part of unit tests of the info module
 */

#include <stdlib.h>
#include <string.h>

#include "cmocka_headers.h"
#include "conn_req.h"
#include "info.h"
#include "librpma.h"
#include "info-common.h"
#include "test-common.h"
#include "mocks-rdma_cm.h"

#include <infiniband/verbs.h>

/*
 * setup__new_passive -- prepare a valid rpma_info object (passive side)
 */
int
setup__new_passive(void **info_state_ptr)
{
	/*
	 * configure mocks for rpma_info_new():
	 * NOTE: it is not allowed to call rdma_freeaddrinfo() if
	 * rdma_getaddrinfo() succeeded.
	 */
	static struct info_state istate;
	memset(&istate, 0, sizeof(istate));
	istate.rai.ai_src_addr = MOCK_SRC_ADDR;
	istate.rai.ai_dst_addr = MOCK_DST_ADDR;
	struct rdma_addrinfo_args args = {MOCK_VALIDATE, &istate.rai};
	will_return(rdma_getaddrinfo, &args);
	expect_value(rdma_getaddrinfo, hints->ai_flags, RAI_PASSIVE);
	will_return(__wrap__test_malloc, MOCK_OK);

	int ret = rpma_info_new(MOCK_IP_ADDRESS, MOCK_PORT, RPMA_INFO_PASSIVE,
			&istate.info);
	assert_int_equal(ret, 0);
	assert_non_null(istate.info);

	*info_state_ptr = &istate;

	return 0;
}

/*
 * setup__new_active -- prepare a valid rpma_info object (active side)
 */
int
setup__new_active(void **info_state_ptr)
{
	/*
	 * configure mocks for rpma_info_new():
	 * NOTE: it is not allowed to call rdma_freeaddrinfo() if
	 * rdma_getaddrinfo() succeeded.
	 */
	static struct info_state istate;
	memset(&istate, 0, sizeof(istate));
	istate.rai.ai_src_addr = MOCK_SRC_ADDR;
	istate.rai.ai_dst_addr = MOCK_DST_ADDR;
	struct rdma_addrinfo_args args = {MOCK_VALIDATE, &istate.rai};
	will_return(rdma_getaddrinfo, &args);
	expect_value(rdma_getaddrinfo, hints->ai_flags, 0);
	will_return(__wrap__test_malloc, MOCK_OK);

	int ret = rpma_info_new(MOCK_IP_ADDRESS, MOCK_PORT, RPMA_INFO_ACTIVE,
			&istate.info);
	assert_int_equal(ret, 0);
	assert_non_null(istate.info);

	*info_state_ptr = &istate;

	return 0;
}

/*
 * teardown__delete -- delete the rpma_info object
 * (either active or passive side)
 */
int
teardown__delete(void **info_state_ptr)
{
	struct info_state *istate = *info_state_ptr;

	/*
	 * configure mocks for rdma_freeaddrinfo():
	 * NOTE: it is not allowed to call rdma_freeaddrinfo() nor malloc() in
	 * rpma_info_delete().
	 */
	struct rdma_addrinfo_args args = {MOCK_VALIDATE, &istate->rai};
	will_return(rdma_freeaddrinfo, &args);

	/* teardown */
	int ret = rpma_info_delete(&istate->info);
	assert_int_equal(ret, MOCK_OK);
	assert_null(istate->info);

	return 0;
}
