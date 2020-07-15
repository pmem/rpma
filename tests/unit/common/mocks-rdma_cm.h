/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2020, Intel Corporation
 */

/*
 * mocks-rdma_cm.h -- a librdmacm mocks header
 */

#ifndef MOCKS_RDMA_CM_H
#define MOCKS_RDMA_CM_H

#define CM_EVENT_CONNECTION_REQUEST_INIT \
	{NULL, NULL, RDMA_CM_EVENT_CONNECT_REQUEST, 0, {{0}}}

#define CM_EVENT_CONNECT_ERROR_INIT \
	{NULL, NULL, RDMA_CM_EVENT_CONNECT_ERROR, 0, {{0}}}

/* mock control entities */

#define MOCK_CTRL_DEFER		1
#define MOCK_CTRL_NO_DEFER	0

/*
 * Cmocka does not allow call expect_* from setup whereas check_* will be called
 * on teardown. So, function creating an object which is called during setup
 * cannot queue any expect_* regarding the function destroying the object
 * which will be called in the teardown.
 */
extern int Mock_ctrl_defer_destruction;

#endif /* MOCKS_RDMA_CM_H */
