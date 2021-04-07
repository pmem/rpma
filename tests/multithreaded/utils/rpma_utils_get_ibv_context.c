// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020-2021, Intel Corporation */

/*
 * rpma_utils-get_ibv_context.c -- 'get ibv context' multithreaded test
 */

#include <librpma.h>

#include "mt_run.h"

struct thread_args {
	/* shared */
	char *addr;
};

/*
 * get_ibv_context__thread_init -- pass a thread-dedicated copy of the pointer
 * to the network interface address
 *
 * Note: The string storing the address is shared between all threads.
 */
static int
get_ibv_context__thread_init(struct mt_test_cli *args, void *thread_args)
{
	struct thread_args *ta = (struct thread_args *)thread_args;
	ta->addr = args->addr;

	return 0;
}

/*
 * get_ibv_context__thread -- try to get an ibv_context based on a shared
 * network interface address string
 */
static void
get_ibv_context__thread(void *arg, struct mt_test_thread_result *tr)
{
	struct thread_args *ta = (struct thread_args *)arg;
	struct ibv_context *dev = NULL;
	int ret;

	/* obtain an IBV context for a local IP address */
	ret = rpma_utils_get_ibv_context(ta->addr,
			RPMA_UTIL_IBV_CONTEXT_LOCAL, &dev);
	if (ret) {
		MT_TEST_THREAD_RPMA_ERR(tr, "rpma_utils_get_ibv_context",
				ret);
	}
}

int
main(int argc, char *argv[])
{
	struct mt_test test = {
			sizeof(struct thread_args),
			get_ibv_context__thread_init,
			NULL,
			get_ibv_context__thread
	};

	return mt_run_test(argc, argv, &test);
}
