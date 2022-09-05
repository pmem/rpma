// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2021-2022, Intel Corporation */

/*
 * rpma_conn_cfg_set_timeout.c -- rpma_conn_cfg_set_timeout multithreaded test
 */

#include <stdlib.h>
#include <librpma.h>

#include "mtt.h"
#include "rpma_conn_cfg_common.h"

struct rpma_conn_cfg_common_prestate prestate = {NULL};

#define TIMEOUT_MS_EXP 2000

struct state {
	struct rpma_conn_cfg *cfg_ptr;
};

/*
 * init -- allocate state and create a new connection configuration object
 */
void
init(unsigned id, void *prestate, void **state_ptr, struct mtt_result *tr)
{
	struct state *st = (struct state *)calloc(1, sizeof(struct state));
	int ret;

	if (!st) {
		MTT_ERR(tr, "calloc", errno);
		return;
	}

	ret = rpma_conn_cfg_new(&st->cfg_ptr);
	if (ret) {
		MTT_RPMA_ERR(tr, "rpma_conn_cfg_new", ret);
		return;
	}

	*state_ptr = st;
}

/*
 * thread -- set connection establishment timeout and check if its value is as expected
 */
static void
thread(unsigned id, void *prestate, void *state, struct mtt_result *tr)
{
	struct rpma_conn_cfg_common_prestate *pr = (struct rpma_conn_cfg_common_prestate *)prestate;
	int ret, timeout_ms;

	ret = rpma_conn_cfg_set_timeout(pr->cfg_ptr, TIMEOUT_MS_EXP);
	if (ret) {
		MTT_RPMA_ERR(tr, "rpma_conn_cfg_set_timeout", ret);
		return;
	}

	ret = rpma_conn_cfg_get_timeout(pr->cfg_ptr, &timeout_ms);
	if (ret) {
		MTT_RPMA_ERR(tr, "rpma_conn_cfg_set_timeout", ret);
		return;
	}

	if (timeout_ms != TIMEOUT_MS_EXP)
		MTT_ERR(tr, "timeout_ms != TIMEOUT_MS_EXP", EINVAL);
}

/*
 * fini -- free the connection configuration object and free the state
 */
static void
fini(unsigned id, void *prestate, void **state_ptr, struct mtt_result *tr)
{
	struct state *st = (struct state *)*state_ptr;
	int ret;

	ret = rpma_conn_cfg_delete(&st->cfg_ptr);
	if (ret)
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

	struct mtt_test test = {
			&prestate,
			rpma_conn_cfg_common_prestate_init,
			NULL,
			init,
			thread,
			fini,
			NULL,
			rpma_conn_cfg_common_prestate_fini
	};

	return mtt_run(&test, args.threads_num);
}
