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
#include <mtt.h>

struct prestate {
	char *addr;
	struct ibv_context *dev;
	int is_odp_supported;
};

struct state {
	int is_odp_supported_exp;
};

/*
 * prestate_init -- obtain an ibv_context for a local IP address
 * and check if the device context's capability supports On-Demand Paging
 */
static void
prestate_init(void *prestate, struct mtt_result *tr)
{
	struct prestate *pr = (struct prestate *)prestate;
	int ret;

	/* obtain an IBV context for a remote IP address */
	ret = rpma_utils_get_ibv_context(pr->addr,
			RPMA_UTIL_IBV_CONTEXT_REMOTE, &pr->dev);
	if (ret) {
		MTT_RPMA_ERR(tr, "rpma_utils_get_ibv_context", ret);
		return;
	}

	/* check if the device context's capability supports On-Demand Paging */
	ret = rpma_utils_ibv_context_is_odp_capable(pr->dev,
				&pr->is_odp_supported);
	if (ret) {
		MTT_RPMA_ERR(tr, "rpma_utils_ibv_context_is_odp_capable", ret);
		return;
	}
}

/*
 * init -- allocate state
 */
void
init(unsigned id, void *prestate, void **state_ptr, struct mtt_result *tr)
{
	struct state *st = (struct state *)calloc(1, sizeof(struct state));
	if (!st) {
		MTT_ERR(tr, "calloc", errno);
		return;
	}

	*state_ptr = st;
}

/*
 * thread -- obtain an ibv_context for a remote IP address
 * and check if the device context's capability supports On-Demand Paging
 */
static void
thread(unsigned id, void *prestate, void *state, struct mtt_result *tr)
{
	struct prestate *pr = (struct prestate *)prestate;
	struct state *st = (struct state *)state;
	struct ibv_context *dev;

	/* obtain an IBV context for a remote IP address */
	int ret = rpma_utils_get_ibv_context(pr->addr,
			RPMA_UTIL_IBV_CONTEXT_REMOTE, &dev);
	if (ret) {
		MTT_RPMA_ERR(tr, "rpma_utils_get_ibv_context", ret);
		return;
	}

	/* check if the device context's capability supports On-Demand Paging */
	ret = rpma_utils_ibv_context_is_odp_capable(dev,
				&st->is_odp_supported_exp);
	if (ret) {
		MTT_RPMA_ERR(tr, "rpma_utils_ibv_context_is_odp_capable", ret);
		return;
	} else if (st->is_odp_supported_exp != pr->is_odp_supported) {
		fprintf(stderr,
				"Unexpected value: is_odp_supported = %d, is_odp_supported_exp = %d\n",
			pr->is_odp_supported, st->is_odp_supported_exp);
		return;
	}
}

/*
 * fini -- free the state
 */
static void
fini(unsigned id, void *prestate, void **state_ptr, struct mtt_result *tr)
{
	struct state *st = (struct state *)*state_ptr;
	free(st);
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
			init,
			thread,
			fini,
			NULL,
			NULL
	};
	return mtt_run(&test, args.threads_num);
}
