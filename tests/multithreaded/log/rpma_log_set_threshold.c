// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2022, Intel Corporation */

/*
 * rpma_log_set_threshold.c -- rpma_log_set_threshold multithreaded test
 */

#include <librpma.h>

#include "mtt.h"

/*
 * thread -- set the log level threshold
 */
void
thread(unsigned id, void *prestate, void *state, struct mtt_result *tr)
{
	int ret = rpma_log_set_threshold(RPMA_LOG_THRESHOLD, RPMA_LOG_LEVEL_INFO);
	if (ret) {
		MTT_RPMA_ERR(tr, "rpma_log_set_threshold", ret);
		return;
	}
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
			NULL,
			thread,
			NULL,
			NULL,
			NULL
	};

	return mtt_run(&test, args.threads_num);
}
