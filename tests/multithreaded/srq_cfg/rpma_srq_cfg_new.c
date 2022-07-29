// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2022, Intel Corporation */

/*
 * rpma_srq_cfg_new.c -- rpma_srq_cfg_new multithreaded test
 */

#include <stdlib.h>
#include <librpma.h>

#include "mtt.h"

struct state {
	struct rpma_srq_cfg *cfg;
};

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
 * thread -- create a new srq configuration object
 */
static void
thread(unsigned id, void *prestate, void *state, struct mtt_result *tr)
{
	struct state *st = (struct state *)state;
	int ret;

	if ((ret = rpma_srq_cfg_new(&st->cfg))) {
		MTT_RPMA_ERR(tr, "rpma_srq_cfg_new", ret);
		return;
	}
}

/*
 * fini -- free the srq configuration object and free the state
 */
static void
fini(unsigned id, void *prestate, void **state_ptr, struct mtt_result *tr)
{
	struct state *st = (struct state *)*state_ptr;
	int ret;

	if ((ret = rpma_srq_cfg_delete(&st->cfg)))
		MTT_RPMA_ERR(tr, "rpma_srq_cfg_delete", ret);

	free(st);
	*state_ptr = NULL;
}

int
main(int argc, char *argv[])
{
	struct mtt_args args = {0};

	if (mtt_parse_args(argc, argv, &args))
		return -1;

	struct mtt_test test = {
			NULL,
			NULL,
			NULL,
			init,
			thread,
			fini,
			NULL,
			NULL
	};

	return mtt_run(&test, args.threads_num);
}
