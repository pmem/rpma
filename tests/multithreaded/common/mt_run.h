/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright 2021, Intel Corporation */

/*
 * mt_run.h -- multithreaded test runner
 */

#ifndef MT_RUN
#define MT_RUN

#include "mt_test.h"

struct mt_run_state {
	struct mt_test_cli args;

	unsigned threads_num_to_join;
	unsigned threads_num_to_fini;
};

int mt_run_test(int argc, char *argv[], struct mt_test *test);

#endif /* MT_RUN */
