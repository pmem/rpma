/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright 2021-2022, Intel Corporation */

/*
 * rpma_conn_cfg_common.h -- common definition for rpma_conn_cfg_*
 * multithreaded tests
 */

#ifndef MTT_RPMA_CONN_CFG_COMMON
#define MTT_RPMA_CONN_CFG_COMMON

/* the expected queue size */
#define RPMA_CONN_CFG_COMMON_Q_SIZE_EXP 20

/* the expected timeout */
#define RPMA_CONN_CFG_COMMON_TIMEOUT_MS_EXP 2000

/* the expected completion channel state */
#define RPMA_CONN_CFG_COMMON_IS_SHARED true

/* the expected SRQ object */
#define RPMA_SRQ_EXP (struct rpma_srq *)0xCD12

struct rpma_conn_cfg_common_prestate {
	struct rpma_conn_cfg *cfg_ptr;
};

void rpma_conn_cfg_common_prestate_init(void *prestate, struct mtt_result *tr);

void rpma_conn_cfg_common_prestate_fini(void *prestate, struct mtt_result *tr);

struct rpma_conn_cfg_common_state {
	struct rpma_conn_cfg *cfg_ptr;
};
#endif /* MTT_RPMA_CONN_CFG_COMMON */
