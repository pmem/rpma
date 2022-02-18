// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2021, Intel Corporation */

/*
 * rpma_utils_ibv_context_is_odp_capable.c -- 'check odp' multithreaded test
 */

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>

#include <librpma.h>

#include "mtt.h"

struct prestate {
	char *addr;
	struct ibv_context *ibv_ctx;
	int is_odp_supported_exp;
};

/*
 * prestate_init -- obtain an ibv_context for a local IP address
 * and check if the device supports On-Demand Paging
 */
static void
prestate_init(void *prestate, struct mtt_result *tr)
{
	struct prestate *pr = (struct prestate *)prestate;
	int ret;

	/* obtain an IBV context for a remote IP address */
	ret = rpma_utils_get_ibv_context(pr->addr,
			RPMA_UTIL_IBV_CONTEXT_REMOTE, &pr->ibv_ctx);
	if (ret) {
		MTT_RPMA_ERR(tr, "rpma_utils_get_ibv_context", ret);
		return;
	}

	/* check if the device supports On-Demand Paging */
	ret = rpma_utils_ibv_context_is_odp_capable(pr->ibv_ctx,
				&pr->is_odp_supported_exp);
	if (ret)
		MTT_RPMA_ERR(tr, "rpma_utils_ibv_context_is_odp_capable", ret);
}

/*
 * thread -- obtain an ibv_context for a remote IP address
 * and check if the device supports On-Demand Paging
 */
static void
thread(unsigned id, void *prestate, void *state, struct mtt_result *tr)
{
	struct prestate *pr = (struct prestate *)prestate;
	int is_odp_supported;

	/* check if the device supports On-Demand Paging */
	int ret = rpma_utils_ibv_context_is_odp_capable(pr->ibv_ctx,
				&is_odp_supported);
	if (ret) {
		MTT_RPMA_ERR(tr, "rpma_utils_ibv_context_is_odp_capable", ret);
	} else if (is_odp_supported != pr->is_odp_supported_exp) {
		MTT_ERR(tr, "is_odp_supported != is_odp_supported_exp", EINVAL);
	}
}

int
main(int argc, char *argv[])
{
	struct mtt_args args = {0};

	if (mtt_parse_args(argc, argv, &args))
		return -1;

	struct prestate prestate = {args.addr, NULL, 0};

	struct mtt_test test = {
			&prestate,
			prestate_init,
			NULL,
			NULL,
			thread,
			NULL,
			NULL,
			NULL
	};

	return mtt_run(&test, args.threads_num);
}
