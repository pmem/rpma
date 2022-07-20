// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2022, Intel Corporation */

/*
 * rpma_log_set_function.c -- rpma_log_set_function multithreaded test
 */

#include <librpma.h>

#include "../src/log_default.h"
#include "mtt.h"

struct prestate {
	rpma_log_function *log_function;
	rpma_log_function *default_log_function;
};

extern rpma_log_function *Rpma_log_function;

/*
 * prestate_init -- assing an rpma_log_function pointer
 */
static void
prestate_init(void *prestate, struct mtt_result *tr)
{
	struct prestate *pr = (struct prestate *)prestate;

	pr->log_function = Rpma_log_function;
	pr->default_log_function = rpma_log_default_function;
}
/*
 * thread -- set log function and check if its value is as expected
 */
void
thread(unsigned id, void *prestate, void *state, struct mtt_result *tr)
{
	struct prestate *pr = (struct prestate *)prestate;
	int ret;

	if ((ret = rpma_log_set_function(RPMA_LOG_USE_DEFAULT_FUNCTION))) {
		MTT_RPMA_ERR(tr, "rpma_log_set_function", ret);
		return;
	}

	if (pr->log_function != pr->default_log_function) {
		MTT_ERR(tr, "Rpma_log_function != rpma_log_default_function", EINVAL);
	}
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
			NULL
	};

	return mtt_run(&test, args.threads_num);
}
