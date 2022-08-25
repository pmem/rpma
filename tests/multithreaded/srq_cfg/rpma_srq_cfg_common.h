/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright 2022, Intel Corporation */

/*
 * rpma_srq_cfg_common.h -- common definition for rpma_srq_cfg_* multithreaded tests
 */

#ifndef MTT_RPMA_SRQ_CFG_COMMON
#define MTT_RPMA_SRQ_CFG_COMMON

/* the expected queue size */
#define RPMA_SRQ_CFG_COMMON_Q_SIZE_EXP 20

void thread(unsigned id, void *prestate, void *state, struct mtt_result *tr);

struct rpma_srq_cfg_common_prestate {
	struct rpma_srq_cfg *cfg_ptr;
};

void rpma_srq_cfg_common_prestate_init(void *prestate, struct mtt_result *tr);

void rpma_srq_cfg_common_prestate_fini(void *prestate, struct mtt_result *tr);

struct rpma_srq_cfg_common_state {
	struct rpma_srq_cfg *cfg_ptr;
};
#endif /* MTT_RPMA_SRQ_CFG_COMMON */
