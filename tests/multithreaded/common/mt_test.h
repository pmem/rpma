/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright 2021, Intel Corporation */

/*
 * mt_test.h -- multithreaded tests' common
 */

#ifndef MT_TEST
#define MT_TEST

#include <stddef.h>

struct mt_test_state {
	/* arguments coming from the command line */
	unsigned threads_num;
	char *addr;

	/* derivatives - modifiable at runtime */
	unsigned threads_num_to_join;
	unsigned threads_num_to_fini;
};

/* define-a-MT-test API */

#define MT_TEST_ERRMSG_MAX 512

/*
 * a store for any thread error message
 *
 * Note: It has to be a part of each thread's specific data to:
 *       - maintain a thread-dedicated copy of it
 *       - allow returning it easily from the working thread to the main thread
 *         on error without any uncertainty about its ownership or validity
 *         in both context
 */
struct mt_test_thread_ret {
	char errmsg[MT_TEST_ERRMSG_MAX];
};

/* on librpma error populate the error string and return from the thread */
#define MT_TEST_THREAD_RPMA_ERR(ta, func, ret) \
	do { \
		snprintf(ta->ret.errmsg, MT_TEST_ERRMSG_MAX - 1, \
			"%s() failed: %s", func, rpma_err_2str(ret)); \
		pthread_exit(&ta->ret); \
	} while (0)

typedef int (*mt_test_thread_args_init)(struct mt_test_state *args,
		void *thread_args);
typedef int (*mt_test_thread_args_fini)(void *thread_args);
typedef void *(*mt_test_thread_func)(void *arg);

struct mt_test {
	/* size of data to be maintained for each of the threads */
	size_t thread_args_size;

	/* a function called for each of threads before spawning it */
	mt_test_thread_args_init thread_init_func;

	/* a function called for each of threads after its termination */
	mt_test_thread_args_fini thread_fini_func;

	/* a thread main function */
	mt_test_thread_func thread_func;
};

int mt_test_run(int argc, char *argv[], struct mt_test *test);

#endif /* MT_TEST */
