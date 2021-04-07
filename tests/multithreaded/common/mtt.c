// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2021, Intel Corporation */

/*
 * mtt.c -- a multithreaded tests' runner
 */

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>

#include <librpma.h>

#include "mtt.h"

#define TIMEOUT_SECONDS 15

static struct {
	/* mutex and conditional used to start all threads synchronously */
	pthread_mutex_t mtx;
	pthread_cond_t cond;
	volatile unsigned threads_num_waiting;

	struct timespec timeout;
} mtt_sync;

struct mtt_thread_args {
	unsigned id; /* a thread id */
	void *state; /* a thread-specific state */
	struct mtt_test *test;
	struct mtt_thread_result ret; /* a thread return object */
};

/*
 * mtt_thread_main -- wait for the synchronization conditional and run the test
 */
static void *
mtt_thread_main(void *arg)
{
	struct mtt_thread_args *ta = (struct mtt_thread_args *)arg;
	struct mtt_test *test = ta->test;
	struct mtt_thread_result *tr = &ta->ret;
	struct mtt_thread_result tr_dummy = {0};
	int result;

	if (test->thread_init_func) {
		test->thread_init_func(ta->id, test->prestate, &ta->state, tr);
		if (tr->ret) {
			/* unblock the main thread waiting for this one */
			++mtt_sync.threads_num_waiting;
			return tr;
		}
	}

	/* wait for all threads to start */
	result = pthread_mutex_lock(&mtt_sync.mtx);
	if (result) {
		MTT_THREAD_ERR(tr, "pthread_mutex_lock", result);
		return tr;
	}

	++mtt_sync.threads_num_waiting;

	result = pthread_cond_timedwait(&mtt_sync.cond, &mtt_sync.mtx,
			&mtt_sync.timeout);
	if (result) {
		MTT_THREAD_ERR(tr, "pthread_cond_timedwait", result);
		(void) pthread_mutex_unlock(&mtt_sync.mtx);
		return tr;
	}
	if ((result = pthread_mutex_unlock(&mtt_sync.mtx))) {
		MTT_THREAD_ERR(tr, "pthread_mutex_unlock", result);
		return tr;
	}

	ta->test->thread_func(ta->id, test->prestate, ta->state, tr);

	if (test->thread_fini_func) {
		/*
		 * if the thread result is already non-zero provide tr_dummy
		 * instead to avoid overwriting the result
		 */
		test->thread_fini_func(&ta->state,
				(tr->ret ? tr : &tr_dummy));
	}

	return tr;
}

/* print an error message for errors not related to any specific thread */
#define MTT_ERR(fmt, ...) \
	fprintf(stderr, "error: " fmt "\n", ##__VA_ARGS__)

/*
 * mtt_threads_sync_unblock -- wait for threads to spawn and unblock all
 * threads synchronously
 */
static int
mtt_threads_sync_unblock(unsigned threads_num)
{
	int ret;
	int done = 0;

	do {
		if ((ret = pthread_mutex_lock(&mtt_sync.mtx))) {
			MTT_ERR("pthread_mutex_lock() failed: %s",
					strerror(ret));
			return ret;
		}

		if (mtt_sync.threads_num_waiting == threads_num) {
			ret = pthread_cond_broadcast(&mtt_sync.cond);
			if (ret) {
				MTT_ERR(
					"pthread_cond_broadcast() failed: %s",
					strerror(ret));
			}

			/*
			 * If broadcasting has failed the waiting threads will
			 * time out so the test will exit with appropriate
			 * error messages. Nothing more can be done.
			 */
			done = 1;
		}

		if ((ret = pthread_mutex_unlock(&mtt_sync.mtx))) {
			MTT_ERR("pthread_mutex_unlock() failed: %s",
					strerror(ret));
			return ret;
		}
	} while (!done);

	return ret;
}

/*
 * mtt_init -- initialize the global state
 */
static int
mtt_init()
{
	int ret;

	if ((ret = pthread_mutex_init(&mtt_sync.mtx, NULL))) {
		MTT_ERR("pthread_mutex_init() failed: %s", strerror(ret));
		return ret;
	}
	if ((ret = pthread_cond_init(&mtt_sync.cond, NULL))) {
		MTT_ERR("pthread_cond_init() failed: %s", strerror(ret));
		(void) pthread_mutex_destroy(&mtt_sync.mtx);
		return ret;
	}

	if ((ret = clock_gettime(CLOCK_REALTIME, &mtt_sync.timeout))) {
		MTT_ERR("clock_gettime() failed: %s", strerror(errno));
		(void) pthread_cond_destroy(&mtt_sync.cond);
		(void) pthread_mutex_destroy(&mtt_sync.mtx);
		return ret;
	}

	mtt_sync.timeout.tv_sec += TIMEOUT_SECONDS;
	mtt_sync.threads_num_waiting = 0;

	return 0;
}

/*
 * mtt_fini -- clean up the global state
 */
static int
mtt_fini()
{
	int ret;
	int result = 0;

	if ((ret = pthread_mutex_destroy(&mtt_sync.mtx)))
		result = ret;
	if ((ret = pthread_cond_destroy(&mtt_sync.cond)))
		result = ret;

	return result;
}

/*
 * mtt_parse_args -- process the command line arguments
 * RETURN -1 on error
 */
int
mtt_parse_args(int argc, char *argv[], struct mtt_args *args)
{
	if (argc < 3) {
		fprintf(stderr, "usage: %s <threads_num> <addr>\n", argv[0]);
		return -1;
	}

	args->threads_num = strtoul(argv[1], NULL, 10);
	args->addr = argv[2];

	return 0;
}

/* print an error message prepended with thread's number */
#define MT_TEST_ERR(thread_num, fmt, ...) \
	fprintf(stderr, "[thread #%d] error: " fmt "\n", \
			thread_num, ##__VA_ARGS__)

/*
 * mtt_run -- run the provided test using provided number of threads
 */
int
mtt_run(struct mtt_test *test, unsigned threads_num)
{
	pthread_t *threads;
	struct mtt_thread_args *threads_args;
	struct mtt_thread_args *ta;
	struct mtt_thread_result *tr;
	unsigned threads_num_to_join = 0;
	unsigned threads_num_to_fini = 0;

	int ret;
	unsigned i;
	int result = 0;

	/* configure logging thresholds to see more details */
	rpma_log_set_threshold(RPMA_LOG_THRESHOLD, RPMA_LOG_LEVEL_INFO);
	rpma_log_set_threshold(RPMA_LOG_THRESHOLD_AUX, RPMA_LOG_LEVEL_INFO);

	/* allocate threads and their arguments */
	threads = calloc(threads_num, sizeof(pthread_t));
	if (threads == NULL) {
		MTT_ERR("calloc failed");
		return -1;
	}
	threads_args = calloc(threads_num, sizeof(struct mtt_thread_args));
	if (threads_args == NULL) {
		MTT_ERR("calloc failed");
		result = -1;
		goto err_free_threads;
	}

	/* initialize threads' arguments */
	for (i = 0; i < threads_num; i++) {
		ta = &threads_args[i];
		ta->id = i;
		ta->test = test;

		if (test->thread_seq_init_func) {
			result = test->thread_seq_init_func(ta->id,
					test->prestate, &ta->state);
			if (result)
				goto err_fini_threads_args;
		}

		++threads_num_to_fini;
	}

	/*
	 * The global initialization has to be as close as possible to spawning
	 * threads since it also calculates an absolute timeout value common
	 * for all threads.
	 */
	if ((result = mtt_init()))
		goto err_fini_threads_args;

	/* create threads */
	for (i = 0; i < threads_num; i++) {
		result = pthread_create(&threads[i], NULL, mtt_thread_main,
				&threads_args[i]);
		if (result != 0) {
			MT_TEST_ERR(i, "pthread_create() failed: %s",
					strerror(result));
			break;
		}

		++threads_num_to_join;
	}

	if ((ret = mtt_threads_sync_unblock(threads_num_to_join)))
			result = ret;

	/* wait for threads to join */
	for (i = 0; i < threads_num_to_join; i++) {
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

	if ((ret = mtt_fini()))
		result = ret;

err_fini_threads_args:
	/* clean up threads' arguments */
	if (test->thread_seq_fini_func) {
		for (i = 0; i < threads_num_to_fini; i++) {
			ret = test->thread_seq_fini_func(
					&threads_args[i].state);
			if (ret)
				result = ret;
		}
	}

	free(threads_args);

err_free_threads:
	free(threads);

	return result;
}
