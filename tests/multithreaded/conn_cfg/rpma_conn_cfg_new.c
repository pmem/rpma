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

struct conn_cfg_values {
	int timeout_ms;
	uint32_t cq_size;
	uint32_t sq_size;
	uint32_t rq_size;
};

static int
conn_cfg_get_all(struct rpma_conn_cfg *cfg, struct conn_cfg_values *vals,
		struct mtt_result *tr)
{
	int ret;

	if ((ret = rpma_conn_cfg_get_timeout(cfg, &vals->timeout_ms))) {
		MTT_RPMA_ERR(tr, "rpma_conn_cfg_get_timeout", ret);
		return -1;
	}

	if ((ret = rpma_conn_cfg_get_cq_size(cfg, &vals->cq_size))) {
		MTT_RPMA_ERR(tr, "rpma_conn_cfg_get_cq_size", ret);
		return -1;
	}

	if ((ret = rpma_conn_cfg_get_sq_size(cfg, &vals->sq_size))) {
		MTT_RPMA_ERR(tr, "rpma_conn_cfg_get_sq_size", ret);
		return -1;
	}

	if ((ret = rpma_conn_cfg_get_rq_size(cfg, &vals->rq_size))) {
		MTT_RPMA_ERR(tr, "rpma_conn_cfg_get_rq_size", ret);
		return -1;
	}

	return 0;
}


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
 * and check its default values
 */
static void
thread(unsigned id, void *prestate, void *state, struct mtt_result *tr)
{
	struct prestate *pr = (struct prestate *)prestate;
	struct state *st = (struct state *)state;
	struct conn_cfg_values cfg_vals;
	struct conn_cfg_values cfg_exp_vals;
	int ret;

	if ((ret = rpma_conn_cfg_new(&st->cfg))) {
		MTT_RPMA_ERR(tr, "rpma_conn_cfg_new", ret);
		return;
	}

	if (conn_cfg_get_all(st->cfg, &cfg_vals, tr))
		return;

	if (conn_cfg_get_all(pr->cfg_exp, &cfg_exp_vals, tr))
		return;

	ret = memcmp(&cfg_vals, &cfg_exp_vals, sizeof(struct conn_cfg_values));
	if (ret)
		MTT_ERR(tr, "cfg_vals != cfg_exp_vals", EINVAL);
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
