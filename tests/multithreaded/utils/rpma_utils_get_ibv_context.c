// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020-2021, Intel Corporation */

/*
 * rpma_utils-get_ibv_context.c -- 'get ibv context' multithreaded test
 */

#include <librpma.h>

#include "mtt.h"

/*
 * get_ibv_context__thread -- try to get an ibv_context based on a shared
 * network interface address string
 */
static void
get_ibv_context__thread(void *prestate, void *thread_state,
		struct mtt_thread_result *tr)
{
	struct mtt_args *args = (struct mtt_args *)prestate;
	struct ibv_context *dev = NULL;
	int ret;

	/* obtain an IBV context for a local IP address */
	ret = rpma_utils_get_ibv_context(args->addr,
			RPMA_UTIL_IBV_CONTEXT_LOCAL, &dev);
	if (ret) {
		MTT_THREAD_RPMA_ERR(tr, "rpma_utils_get_ibv_context",
				ret);
	}
}

int
main(int argc, char *argv[])
{
	struct mtt_args args = {0};

	if (mtt_parse_args(argc, argv, &args))
		return -1;

	struct mtt_test test = {
			&args, /* used as a prestate */
			NULL,
			NULL,
			get_ibv_context__thread,
			NULL,
			NULL
	};

	return mtt_run(&test, args.threads_num);
}
