// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020-2021, Intel Corporation */

/*
 * rpma_utils-get_ibv_context.c -- 'get ibv context' multithreaded test
 */

#include <librpma.h>

#include "mtt.h"

struct prestate {
	char *addr;
};

/*
 * thread -- try to get an ibv_context based on a shared network interface
 * address string
 */
static void
thread(unsigned id, void *prestate, void *state,
		struct mtt_result *result)
{
	struct prestate *ps = (struct prestate *)prestate;
	struct ibv_context *dev = NULL;
	int ret;

	/* obtain an IBV context for a local IP address */
	ret = rpma_utils_get_ibv_context(ps->addr, RPMA_UTIL_IBV_CONTEXT_LOCAL,
			&dev);
	if (ret)
		MTT_RPMA_ERR(result, "rpma_utils_get_ibv_context", ret);
}

int
main(int argc, char *argv[])
{
	struct mtt_args args = {0};

	if (mtt_parse_args(argc, argv, &args))
		return -1;

	struct prestate prestate = {args.addr};

	struct mtt_test test = {
			&prestate,
			NULL,
			NULL,
			NULL,
			thread,
			NULL,
			NULL,
			NULL
	};

	return mtt_run(&test, args.threads_num);
}
