// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2021, Intel Corporation */

/*
 * example.c -- an example of a multithreaded test
 *
 * An example showing how to use the multithreaded test API (mtt_*).
 * The test itself does a very simple thing of rewriting input to the output
 * using an intermediate temp variable (of course it is nonsensical).
 * The test runs as follow:
 * - example__seq_init - sequentially allocates and initializes states for all
 *                       of the threads
 * - example__init     - in parallel all of the threads allocate a temp variable
 *                       for themselves
 * - example__thread   - rewrite input to the temp variable
 * - example__fini     - in parallel rewrites temp to the output and frees
 *                       the temp variable
 * - example__seq_fini - sequentially prints the output of each of the threads
 *                       and frees their states
 */

#include <errno.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mtt.h"

/*
 * prestate is an object which exists in a single copy and is shared among all
 * threads. It can be safely modified during the sequential steps
 * (thread_seq_init_func, thread_seq_fini_func). Accessing it during parallel
 * steps (thread_init_func, thread_func, thread_fini_func) has to be executed
 * in a thread-safe manner.
 */
struct prestate {
	uint64_t seed;
};

/*
 * state is an object which is allocated for each of the threads. It can be
 * allocated during sequential or parallel initialization step
 * (thread_seq_init_func, thread_init_func) and freed during parallel or
 * sequential cleanup step (thread_fini_func, thread_seq_fini_func).
 * Since it is dedicated to a single thread it can be accessed freely during
 * all test steps.
 */
struct state {
	uint64_t input;
	uint64_t *temp;
	uint64_t output;
};

/*
 * seq_init -- a sequential step of initialization; here should go everything
 * that is not thread-safe and/or should be prepared in a sequential manner
 */
static void
seq_init(unsigned id, void *prestate, void **state_ptr,
		struct mtt_result *tr)
{
	struct prestate *pr = (struct prestate *)prestate;
	struct state *st = (struct state *)calloc(1, sizeof(struct state));
	if (!st) {
		MTT_ERR(tr, "calloc", errno);
		return;
	}

	st->input = pr->seed + id;
	*state_ptr = st;
}

/*
 * init -- a parallel step of initialization; here should go everything that is
 * thread-safe and makes preparation for the test execution
 */
void
init(unsigned id, void *prestate, void **state_ptr,
		struct mtt_result *tr)
{
	struct state *st = (struct state *)*state_ptr;
	st->temp = calloc(1, sizeof(uint64_t));
	if (!st->temp) {
		MTT_ERR(tr, "calloc", errno);
		return;
	}
}

/*
 * thread -- a test itself (parallel)
 */
static void
thread(unsigned id, void *prestate, void *state,
		struct mtt_result *tr)
{
	struct state *st = (struct state *)state;
	*st->temp = st->input;
}

/*
 * fini -- a parallel step of cleanup; here should go everything that
 * can be cleaned up after the test in a thread-safe manner
 */
static void
fini(unsigned id, void *prestate, void **state_ptr,
		struct mtt_result *tr)
{
	struct state *st = (struct state *)*state_ptr;
	st->output = *st->temp;

	free(st->temp);
	st->temp = NULL;
}

/*
 * seq_fini -- a sequential step of cleanup; here should go everything that
 * has to be cleaned up after the test but is not thread-safe
 */
static void
seq_fini(unsigned id, void *prestate, void **state_ptr,
		struct mtt_result *tr)
{
	struct state *st = (struct state *)*state_ptr;
	printf("[%u] = %" PRIu64 "\n", id, st->output);

	free(st);
	*state_ptr = NULL;
}

int
main(int argc, char *argv[])
{
	struct mtt_args args = {0};

	if (mtt_parse_args(argc, argv, &args))
		return -1;

	struct prestate prestate = {5}; /* an arbitrary seed */

	struct mtt_test test = {
			&prestate,
			seq_init,
			init,
			thread,
			fini,
			seq_fini
	};

	return mtt_run(&test, args.threads_num);
}
