// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2021, Intel Corporation */

/*
 * rpma_conn_cfg_common.h -- common definition for rpma_conn_cfg_xxx mtt
 */

#ifndef RPMA_CONN_CFG_COMMON
#define RPMA_CONN_CFG_COMMON

#include <librpma.h>

/* the expected queue size */
#define RPMA_CONN_CFG_COMMON_Q_SIZE_EXP 20

struct state {
	struct rpma_conn_cfg *cfg_ptr;
};

void
rpma_conn_cfg_common_init(unsigned id, void *prestate, void **state_ptr,
		struct mtt_result *tr);

void
rpma_conn_cfg_common_fini(unsigned id, void *prestate, void **state_ptr,
		struct mtt_result *tr);

#endif /* RPMA_CONN_CFG_COMMON */
