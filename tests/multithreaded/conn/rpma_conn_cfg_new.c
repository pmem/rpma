// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2021, Intel Corporation */

/*
 * rpma_conn_cfg_new.c -- rpma_conn_cfg_new multithreaded test
 */

#include <stdlib.h>
#include <librpma.h>

#include "mtt.h"

struct rpma_conn_cfg {
	int timeout_ms;	/* connection establishment timeout */
	uint32_t cq_size;	/* CQ size */
	uint32_t sq_size;	/* SQ size */
	uint32_t rq_size;	/* RQ size */
};

struct prestate {
	struct rpma_conn_cfg *cfg_ptr_exp;
};

struct state {
	struct rpma_conn_cfg *cfg_ptr;
};

/*
 * prestate_init -- creates a new connection configuration object
 */
static void
prestate_init(void *prestate, struct mtt_result *tr)
{
	struct prestate *pr = (struct prestate *)prestate;
	int ret;

	if ((ret = rpma_conn_cfg_new(&pr->cfg_ptr_exp)))
		MTT_RPMA_ERR(tr, "rpma_conn_cfg_new", ret);
}

/*
 * init -- allocate state
 */
void
init(unsigned id, void *prestate, void **state_ptr,
		struct mtt_result *tr)
{
	struct state *st = (struct state *)calloc(1, sizeof(struct state));
	if (!st) {
		MTT_ERR(tr, "calloc", errno);
		return;
	}

	*state_ptr = st;
}

/*
 * thread -- creates a new connection configuration object
 * and check if its default sizes are expected
 */
static void
thread(unsigned id, void *prestate, void *state, struct mtt_result *tr)
{
	struct prestate *pr = (struct prestate *)prestate;
	struct state *st = (struct state *)state;
	int ret;

	if ((ret = rpma_conn_cfg_new(&st->cfg_ptr))) {
		MTT_RPMA_ERR(tr, "rpma_conn_cfg_new", ret);
		return;
	}

	if (st->cfg_ptr->timeout_ms != pr->cfg_ptr_exp->timeout_ms ||
		st->cfg_ptr->cq_size != pr->cfg_ptr_exp->cq_size ||
		st->cfg_ptr->sq_size != pr->cfg_ptr_exp->sq_size ||
		st->cfg_ptr->rq_size != pr->cfg_ptr_exp->rq_size)
		MTT_ERR(tr, "rpma_conn_cfg_new create an unexpected value",
			EINVAL);
}

/*
 * fini -- free the state
 */
static void
fini(unsigned id, void *prestate, void **state_ptr,
		struct mtt_result *tr)
{
	struct state *st = (struct state *)*state_ptr;
	int ret;

	if ((ret = rpma_conn_cfg_delete(&st->cfg_ptr)))
		MTT_RPMA_ERR(tr, "rpma_conn_cfg_delete", ret);

	free(st);
	*state_ptr = NULL;
}

int
main(int argc, char *argv[])
{
	struct mtt_args args = {0};

	if (mtt_parse_args(argc, argv, &args))
		return -1;

	struct prestate prestate = {NULL};

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
