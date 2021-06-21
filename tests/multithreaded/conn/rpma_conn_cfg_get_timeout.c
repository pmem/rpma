// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2021, Intel Corporation */

/*
 * rpma_conn_cfg_get_rq_size.c -- rpma_conn_cfg_get_rq_size multithreaded test
 */

#include <stdlib.h>
#include <librpma.h>

#include "mtt.h"

#define TIMEOUT_MS_EXP 2000

struct prestate {
	struct rpma_conn_cfg *cfg_ptr;
};

/*
 * prestate_init -- create a new connection configuration object
 */
static void
prestate_init(void *prestate, struct mtt_result *tr)
{
	struct prestate *pr = (struct prestate *)prestate;
	int ret;

	if ((ret = rpma_conn_cfg_new(&pr->cfg_ptr))) {
		MTT_RPMA_ERR(tr, "rpma_conn_cfg_new", ret);
		return;
	}

	if ((ret = rpma_conn_cfg_set_timeout(pr->cfg_ptr, TIMEOUT_MS_EXP)))
		MTT_RPMA_ERR(tr, "rpma_conn_cfg_set_timeout", ret);
}

/*
 * thread -- get connection configured timeout value and check if its value is
 * as expected
 */
static void
thread(unsigned id, void *prestate, void *state, struct mtt_result *tr)
{
	struct prestate *pr = (struct prestate *)prestate;
	int ret, timeout_ms;

	if ((ret = rpma_conn_cfg_get_timeout(pr->cfg_ptr, &timeout_ms))) {
		MTT_RPMA_ERR(tr, "rpma_conn_cfg_get_timeout", ret);
		return;
	}

	if (timeout_ms != TIMEOUT_MS_EXP)
		MTT_ERR(tr, "timeout_ms != TIMEOUT_MS_EXP", EINVAL);
}

/*
 * prestate_fini -- free the connection configuration object
 */
static void
prestate_fini(void *prestate, struct mtt_result *tr)
{
	struct prestate *pr = (struct prestate *)prestate;
	int ret;

	if ((ret = rpma_conn_cfg_delete(&pr->cfg_ptr)))
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
			NULL,
			thread,
			NULL,
			NULL,
			prestate_fini
	};

	return mtt_run(&test, args.threads_num);
}
