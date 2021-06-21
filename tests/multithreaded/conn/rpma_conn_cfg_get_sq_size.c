// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2021, Intel Corporation */

/*
 * rpma_conn_cfg_get_sq_size.c -- rpma_conn_cfg_get_sq_size multithreaded test
 */

#include <stdlib.h>
#include <librpma.h>

#include "mtt.h"

#define Q_SIZE_EXP 20

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

	if ((ret = rpma_conn_cfg_set_sq_size(pr->cfg_ptr, Q_SIZE_EXP)))
		MTT_RPMA_ERR(tr, "rpma_conn_cfg_set_sq_size", ret);
}

/*
 * thread -- get connection configured sq size and check if its value is
 * as expected
 */
static void
thread(unsigned id, void *prestate, void *state, struct mtt_result *tr)
{
	struct prestate *pr = (struct prestate *)prestate;
	uint32_t sq_size;
	int ret;

	if ((ret = rpma_conn_cfg_get_sq_size(pr->cfg_ptr, &sq_size))) {
		MTT_RPMA_ERR(tr, "rpma_conn_cfg_get_sq_size", ret);
		return;
	}

	if (sq_size != Q_SIZE_EXP)
		MTT_ERR(tr, "sq_size != Q_SIZE_EXP", EINVAL);
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
