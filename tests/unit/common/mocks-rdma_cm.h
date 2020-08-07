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

#define MOCK_EVCH		(struct rdma_event_channel *)&Evch
#define MOCK_CM_ID		(struct rdma_cm_id *)&Cm_id

#define RDMA_MIGRATE_TO_EVCH 0
#define RDMA_MIGRATE_FROM_EVCH 1
#define RDMA_MIGRATE_COUNTER_INIT (RDMA_MIGRATE_TO_EVCH)

extern struct rdma_event_channel Evch; /* mock event channel */
extern struct rdma_cm_id Cm_id;	/* mock CM ID */
extern int Rdma_migrate_id_counter;

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
