// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2021, Intel Corporation */

/*
 * rpma_conn_cfg_new.c -- rpma_conn_cfg_new multithreaded test
 */

#include <stdlib.h>
#include <librpma.h>

#include "mtt.h"

struct prestate {
	struct rpma_conn_cfg *cfg_exp;
};

struct state {
	struct rpma_conn_cfg *cfg;
};

/*
 * prestate_init -- create a new connection configuration object
 */
static void
prestate_init(void *prestate, struct mtt_result *tr)
{
	struct prestate *pr = (struct prestate *)prestate;
	int ret;

	if ((ret = rpma_conn_cfg_new(&pr->cfg_exp)))
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
 * thread -- create a new connection configuration object
 * and check its default sizes->values
 */
static void
thread(unsigned id, void *prestate, void *state, struct mtt_result *tr)
{
	struct prestate *pr = (struct prestate *)prestate;
	struct state *st = (struct state *)state;
	int timeout_ms, timeout_ms_exp;
	uint32_t cq_size, cq_size_exp;
	uint32_t sq_size, sq_size_exp;
	uint32_t rq_size, rq_size_exp;
	int ret;

	if ((ret = rpma_conn_cfg_new(&st->cfg))) {
		MTT_RPMA_ERR(tr, "rpma_conn_cfg_new", ret);
		return;
	}

	if ((ret = rpma_conn_cfg_get_timeout(st->cfg, &timeout_ms))) {
		MTT_RPMA_ERR(tr, "rpma_conn_cfg_get_timeout", ret);
		return;
	}

	if ((ret = rpma_conn_cfg_get_timeout(pr->cfg_exp, &timeout_ms_exp))) {
		MTT_RPMA_ERR(tr, "rpma_conn_cfg_get_timeout", ret);
		return;
	}

	if ((ret = rpma_conn_cfg_get_cq_size(st->cfg, &cq_size))) {
		MTT_RPMA_ERR(tr, "rpma_conn_cfg_get_cq_size", ret);
		return;
	}

	if ((ret = rpma_conn_cfg_get_cq_size(pr->cfg_exp, &cq_size_exp))) {
		MTT_RPMA_ERR(tr, "rpma_conn_cfg_get_cq_size", ret);
		return;
	}

	if ((ret = rpma_conn_cfg_get_sq_size(st->cfg, &sq_size))) {
		MTT_RPMA_ERR(tr, "rpma_conn_cfg_get_sq_size", ret);
		return;
	}

	if ((ret = rpma_conn_cfg_get_sq_size(pr->cfg_exp, &sq_size_exp))) {
		MTT_RPMA_ERR(tr, "rpma_conn_cfg_get_sq_size", ret);
		return;
	}

	if ((ret = rpma_conn_cfg_get_rq_size(st->cfg, &rq_size))) {
		MTT_RPMA_ERR(tr, "rpma_conn_cfg_get_rq_size", ret);
		return;
	}

	if ((ret = rpma_conn_cfg_get_rq_size(pr->cfg_exp, &rq_size_exp))) {
		MTT_RPMA_ERR(tr, "rpma_conn_cfg_get_rq_size", ret);
		return;
	}

	if (timeout_ms != timeout_ms_exp ||	cq_size != cq_size_exp ||
		sq_size != sq_size_exp || rq_size != rq_size_exp)
		MTT_ERR(tr, "cfg != pr->cfg_ptr", EINVAL);
}

/*
 * fini -- free the connection configuration object and free the state
 */
static void
fini(unsigned id, void *prestate, void **state_ptr,
		struct mtt_result *tr)
{
	struct state *st = (struct state *)*state_ptr;
	int ret;

	if ((ret = rpma_conn_cfg_delete(&st->cfg)))
		MTT_RPMA_ERR(tr, "rpma_conn_cfg_delete", ret);

	free(st);
	*state_ptr = NULL;
}

/*
 * prestate_fini -- free the connection configuration object
 */
static void
prestate_fini(void *prestate, struct mtt_result *tr)
{
	struct prestate *pr = (struct prestate *)prestate;
	int ret;

	if ((ret = rpma_conn_cfg_delete(&pr->cfg_exp)))
		MTT_RPMA_ERR(tr, "rpma_conn_cfg_delete", ret);
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
			prestate_fini
	};

	return mtt_run(&test, args.threads_num);
}
