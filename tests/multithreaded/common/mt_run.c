// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2021, Intel Corporation */

/*
 * mt_run.c -- multithreaded tests' runner
 */

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>

#include <librpma.h>

#include "mt_run.h"
#include "mt_test.h"

#define TIMEOUT_SECONDS 15

static struct {
	/* mutex and conditional used to start all threads synchronously */
	pthread_mutex_t mtx;
	pthread_cond_t cond;
	volatile unsigned threads_num_waiting;

	struct timespec timeout;
} mt_test_sync;

struct mt_run_thread_args {
	mt_test_thread_func thread_func; /* a thread main function */
	struct mt_test_thread_result ret; /* a thread return object */

	char thread_args[]; /* a thread-specific arguments */
};

/*
 * mt_run_thread -- wait for the synchronization conditional and run the test
 */
static void *
mt_run_thread(void *arg)
{
	struct mt_run_thread_args *rta = (struct mt_run_thread_args *)arg;
	struct mt_test_thread_result *tr = &rta->ret;
	int result;

	/* wait for all threads to start */
	result = pthread_mutex_lock(&mt_test_sync.mtx);
	if (result) {
		MT_TEST_THREAD_ERR(tr, "pthread_mutex_lock", result);
		return tr;
	}

	++mt_test_sync.threads_num_waiting;

	result = pthread_cond_timedwait(&mt_test_sync.cond, &mt_test_sync.mtx,
			&mt_test_sync.timeout);
	if (result) {
		MT_TEST_THREAD_ERR(tr, "pthread_cond_timedwait", result);
		(void) pthread_mutex_unlock(&mt_test_sync.mtx);
		return tr;
	}
	if ((result = pthread_mutex_unlock(&mt_test_sync.mtx))) {
		MT_TEST_THREAD_ERR(tr, "pthread_mutex_unlock", result);
		return tr;
	}

	rta->thread_func(rta->thread_args, tr);

	return tr;
}

/*
 * print an error message for global errors (not related to any specific thread)
 */
#define MT_TEST_GLOBAL_ERR(fmt, ...) \
	fprintf(stderr, "error: " fmt "\n", ##__VA_ARGS__)

/*
 * mt_run_broadcast_threads -- wait for threads to spawn and unblock all threads
 * synchronously
 */
static int
mt_run_broadcast_threads(struct mt_run_state *state)
{
	int ret;
	int done = 0;

	do {
		if ((ret = pthread_mutex_lock(&mt_test_sync.mtx))) {
			MT_TEST_GLOBAL_ERR("pthread_mutex_lock() failed: %s",
					strerror(ret));
			return ret;
		}

		if (mt_test_sync.threads_num_waiting
				== state->args.threads_num) {
			ret = pthread_cond_broadcast(&mt_test_sync.cond);
			if (ret) {
				MT_TEST_GLOBAL_ERR(
						"pthread_mutex_lock() failed: %s",
						strerror(ret));
			}

			/*
			 * No matter if the threads have been unblocked
			 * successfully or not nothing more can be done.
			 */
			done = 1;
		}

		if ((ret = pthread_mutex_unlock(&mt_test_sync.mtx))) {
			MT_TEST_GLOBAL_ERR("pthread_mutex_unlock() failed: %s",
					strerror(ret));
			return ret;
		}
	} while (!done);

	return ret;
}

/*
 * parse_args -- process the command line arguments and initialize the MT-test
 * runner state according to them
 */
static int
mt_run_parse_args(int argc, char *argv[], struct mt_test_cli *args)
{
	if (argc < 3) {
		fprintf(stderr, "usage: %s <threads_num> <addr>\n", argv[0]);
		return -1;
	}

	args->threads_num = strtoul(argv[1], NULL, 10);
	args->addr = argv[2];

	return 0;
}

/*
 * mt_run_state_init -- initialize the state
 */
static void
mt_run_state_init(struct mt_run_state *state)
{
	/*
	 * If no error occurs the number of threads to be joined and the number
	 * of threads which will require cleanup are equal to the total number
	 * of threads planned.
	 */
	state->threads_num_to_join = state->args.threads_num;
	state->threads_num_to_fini = state->args.threads_num;
}

/*
 * mt_run_global_init -- initialize the global state
 */
static int
mt_run_global_init()
{
	int ret;

	if ((ret = pthread_mutex_init(&mt_test_sync.mtx, NULL))) {
		MT_TEST_GLOBAL_ERR("pthread_mutex_init() failed: %s",
							strerror(ret));
		return ret;
	}
	if ((ret = pthread_cond_init(&mt_test_sync.cond, NULL))) {
		MT_TEST_GLOBAL_ERR("pthread_cond_init() failed: %s",
							strerror(ret));
		(void) pthread_mutex_destroy(&mt_test_sync.mtx);
		return ret;
	}
	if ((ret = clock_gettime(CLOCK_REALTIME, &mt_test_sync.timeout))) {
		MT_TEST_GLOBAL_ERR("clock_gettime() failed: %s",
				strerror(errno));
		(void) pthread_cond_destroy(&mt_test_sync.cond);
		(void) pthread_mutex_destroy(&mt_test_sync.mtx);
		return ret;
	}

	mt_test_sync.timeout.tv_sec += TIMEOUT_SECONDS;
	mt_test_sync.threads_num_waiting = 0;

	return 0;
}

/*
 * mt_run_global_fini -- clean up the global state
 */
static int
mt_run_global_fini()
{
	int ret;
	int result = 0;

	if ((ret = pthread_mutex_destroy(&mt_test_sync.mtx)))
		result = ret;
	if ((ret = pthread_cond_destroy(&mt_test_sync.cond)))
		result = ret;

	return result;
}

/* calculate the total size of thread related data */
#define MT_TEST_THREAD_ARGS_SIZE(test) \
	(sizeof(struct mt_run_thread_args) + (test)->thread_args_size)

/* a helper for extracting a thread related data based on thread's number */
#define MT_TEST_THREAD_ARGS(test, threads_args, thread_num) \
	(struct mt_run_thread_args *) \
		&threads_args[MT_TEST_THREAD_ARGS_SIZE(test) * thread_num]

/* print an error message prepended with thread's number */
#define MT_TEST_ERR(thread_num, fmt, ...) \
	fprintf(stderr, "[thread #%d] error: " fmt "\n", \
			thread_num, ##__VA_ARGS__)

/*
 * mt_run_test -- run the provided mt_test taking into account the command line
 * arguments
 */
int
mt_run_test(int argc, char *argv[], struct mt_test *test)
{
	struct mt_run_state state;

	pthread_t *threads;
	char *threads_args;
	struct mt_run_thread_args *rta;
	struct mt_test_thread_result *tr;

	int ret;
	unsigned i;
	int result = 0;

	/* configure logging thresholds to see more details */
	rpma_log_set_threshold(RPMA_LOG_THRESHOLD, RPMA_LOG_LEVEL_INFO);
	rpma_log_set_threshold(RPMA_LOG_THRESHOLD_AUX, RPMA_LOG_LEVEL_INFO);

	if ((ret = mt_run_parse_args(argc, argv, &state.args)))
		return ret;

	mt_run_state_init(&state);

	/* allocate threads and their arguments */
	threads = calloc(state.args.threads_num, sizeof(pthread_t));
	if (threads == NULL) {
		MT_TEST_GLOBAL_ERR("calloc failed");
		return -1;
	}
	threads_args = calloc(state.args.threads_num,
			MT_TEST_THREAD_ARGS_SIZE(test));
	if (threads_args == NULL) {
		MT_TEST_GLOBAL_ERR("calloc failed");
		result = -1;
		goto err_free_threads;
	}

	/* initialize threads' arguments */
	if (test->thread_init_func) {
		for (i = 0; i < state.args.threads_num; i++) {
			rta = MT_TEST_THREAD_ARGS(test, threads_args, i);
			rta->thread_func = test->thread_func;
			result = test->thread_init_func(&state.args,
				rta->thread_args);
			if (result) {
				state.threads_num_to_fini = i;
				goto err_fini_threads_args;
			}
		}
	}

	/*
	 * The global initialization has to be as close as possible to spawning
	 * threads since it also calculates an absolute timeout value common
	 * for all threads.
	 */
	if ((result = mt_run_global_init(&state)))
		goto err_fini_threads_args;

	/* create threads */
	for (i = 0; i < state.args.threads_num; i++) {
		result = pthread_create(&threads[i], NULL, mt_run_thread,
				MT_TEST_THREAD_ARGS(test, threads_args, i));
		if (result != 0) {
			MT_TEST_ERR(i, "pthread_create() failed: %s",
					strerror(ret));
			state.threads_num_to_join = i;
			break;
		}
	}

	mt_run_broadcast_threads(&state);

	/* wait for threads to join */
	for (i = 0; i < state.threads_num_to_join; i++) {
		ret = pthread_join(threads[i], (void **)&tr);
		if (ret != 0) {
			MT_TEST_ERR(i, "pthread_join() failed: %s",
					strerror(ret));
			result = ret;
		} else if (tr == NULL) {
			MT_TEST_ERR(i, "returned a NULL result");
			result = -1;
		} else if (tr->ret != 0) {
			MT_TEST_ERR(i, "%s", tr->errmsg);
			result = tr->ret;
		}
	}

	if ((ret = mt_run_global_fini()))
		result = ret;

err_fini_threads_args:
	/* clean up threads' arguments */
	if (test->thread_fini_func) {
		for (i = 0; i < state.threads_num_to_fini; i++) {
			rta = MT_TEST_THREAD_ARGS(test, threads_args, i);
			ret = test->thread_fini_func(rta->thread_args);
			if (ret)
				result = ret;
		}
	}

	free(threads_args);

err_free_threads:
	free(threads);

	return result;
}
