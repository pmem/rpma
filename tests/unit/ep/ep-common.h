/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright 2020, Intel Corporation */

/*
 * ep-common.h -- header of the common part of the endpoint unit tests
 */

#ifndef EP_COMMON_H
#define EP_COMMON_H 1

#include <rdma/rdma_cma.h>

#define MOCK_CONN_REQ	(struct rpma_conn_req *)0xCFEF
#define MOCK_FD		0x00FD

/* mock control entities */

#define MOCK_CTRL_DEFER		1
#define MOCK_CTRL_NO_DEFER	0

extern const struct rdma_cm_id Cmid_zero;
extern const struct rdma_event_channel Evch_zero;
extern int Mock_ctrl_defer_destruction;

int setup__ep_listen(void **estate_ptr);
int teardown__ep_shutdown(void **estate_ptr);

/*
 * All the resources used between setup__ep_listen and teardown__ep_shutdown.
 */
struct ep_test_state {
	struct rdma_event_channel evch;
	struct rdma_cm_id cmid;
	struct rpma_ep *ep;

	struct rpma_conn_cfg *cfg;
};

void prestate_init(struct ep_test_state *prestate, struct rpma_conn_cfg *cfg);

#endif /* EP_COMMON_H */
