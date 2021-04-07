// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2021, Intel Corporation */

/*
 * mt_test.c -- multithreaded tests' common
 */

#include "mt_test.h"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>

#include <librpma.h>

#include "mt_test.h"

/*
 * parse_args -- process the command line arguments and initialize the MT-test
 * runner state according to them
 */
static int
parse_args(int argc, char *argv[], struct mt_test_state *state)
{
	if (argc < 3) {
		fprintf(stderr, "usage: %s <threads_num> <addr>\n", argv[0]);
		return -1;
	}

	state->threads_num = strtoul(argv[1], NULL, 10);
	state->addr = argv[2];

	/*
	 * If no error occurs the number of threads to be joined and the number
	 * of threads which will require cleanup are equal to the total number
	 * of threads planned.
	 */
	state->threads_num_to_join = state->threads_num;
	state->threads_num_to_fini = state->threads_num;

	return 0;
}

/* a helper for extracting a thread related data based on thread's number */
#define MT_TEST_THREAD_ARGS(test, threads_args, thread_num) \
	&threads_args[test->thread_args_size * thread_num]

/* print an error message prepended with thread's number */
#define MT_TEST_ERR(thread_num, fmt, ...) \
	fprintf(stderr, "[thread #%d] error: " fmt "\n", \
			thread_num, ##__VA_ARGS__)

/*
 * mt_test_run -- run the provided mt_test taking into account the command line
 * arguments
 */
int
mt_test_run(int argc, char *argv[], struct mt_test *test)
{
	struct mt_test_state state;

	pthread_t *threads;
	char *threads_args;
	struct mt_test_thread_ret *thread_ret;

	int ret;
	unsigned i;
	int result = 0;

	/* configure logging thresholds to see more details */
	rpma_log_set_threshold(RPMA_LOG_THRESHOLD, RPMA_LOG_LEVEL_INFO);
	rpma_log_set_threshold(RPMA_LOG_THRESHOLD_AUX, RPMA_LOG_LEVEL_INFO);

	if ((ret = parse_args(argc, argv, &state)))
		return ret;

	/* allocate threads and their arguments */
	threads = calloc((size_t)state.threads_num, sizeof(pthread_t));
	if (threads == NULL) {
		fprintf(stderr, "error: calloc failed\n");
		return -1;
	}
	threads_args = calloc((size_t)state.threads_num,
			test->thread_args_size);
	if (threads_args == NULL) {
		fprintf(stderr, "error: calloc failed\n");
		result = -1;
		goto err_free_threads;
	}

	/* initialize threads' arguments */
	if (test->thread_init_func) {
		for (i = 0; i < state.threads_num; i++) {
			result = test->thread_init_func(&state,
					MT_TEST_THREAD_ARGS(test, threads_args,
							i));
			if (result) {
				state.threads_num_to_fini = i;
				goto err_fini_threads_args;
			}
		}
	}

	/* create threads */
	for (i = 0; i < state.threads_num; i++) {
		result = pthread_create(&threads[i], NULL, test->thread_func,
				MT_TEST_THREAD_ARGS(test, threads_args, i));
		if (result != 0) {
			MT_TEST_ERR(i, "pthread_create() failed: %s",
					strerror(ret));
			state.threads_num_to_join = i;
			break;
		}
	}

	/* wait for threads to join */
	for (i = 0; i < state.threads_num_to_join; i++) {
		ret = pthread_join(threads[i], (void **)&thread_ret);
		if (ret != 0) {
			MT_TEST_ERR(i, "pthread_join() failed: %s",
					strerror(ret));
			result = ret;
		} else if (thread_ret != NULL) {
			MT_TEST_ERR(i, "%s", thread_ret->errmsg);
			result = -1;
		}
	}

err_fini_threads_args:
	/* clean up threads' arguments */
	if (test->thread_fini_func) {
		for (i = 0; i < state.threads_num_to_fini; i++) {
			ret = test->thread_fini_func(MT_TEST_THREAD_ARGS(test,
					threads_args, i));
			if (ret)
				result = ret;
		}
	}

	free(threads_args);

err_free_threads:
	free(threads);

	return result;
}
