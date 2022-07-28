// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2022, Intel Corporation */

/*
 * rpma_srq_cfg_common_set_get.c -- common main function for all srq_cfg mt set/get tests
 */

#include "mtt.h"
#include "rpma_srq_cfg_common.h"

int
main(int argc, char *argv[])
{
	struct mtt_args args = {0};

	if (mtt_parse_args(argc, argv, &args))
		return -1;

	struct rpma_srq_cfg_common_prestate prestate = {NULL};

	struct mtt_test test = {
			&prestate,
			rpma_srq_cfg_common_prestate_init,
			NULL,
			NULL,
			thread,
			NULL,
			NULL,
			rpma_srq_cfg_common_prestate_fini
	};

	return mtt_run(&test, args.threads_num);
}
