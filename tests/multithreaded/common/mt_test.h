/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright 2021, Intel Corporation */

/*
 * mt_test.h -- define-a-MT-test API
 */

#ifndef MT_TEST
#define MT_TEST

#include <stddef.h>

#define MT_TEST_ERRMSG_MAX 512

/*
 * a store for any thread error message
 *
 * Note: It is the ONLY valid return value from a thread.
 *
 * Note: It has to be a part of each thread's specific data to:
 *       - maintain a thread-dedicated copy of it
 *       - allow returning it easily from the working thread to the main thread
 *         on error without any uncertainty about its ownership or validity
 *         in both contexts
 */
struct mt_test_thread_ret {
	int result;
	char errmsg[MT_TEST_ERRMSG_MAX];
};

/* on librpma error populate the result and the error string */
#define MT_TEST_THREAD_RPMA_ERR(ta, func, ret) \
	do { \
		ta->ret.result = ret; \
		snprintf(ta->ret.errmsg, MT_TEST_ERRMSG_MAX - 1, \
			"%s() failed: %s", func, rpma_err_2str(ret)); \
	} while (0)

/* arguments coming from the command line */
struct mt_test_cli {
	unsigned threads_num;
	char *addr;
};

typedef int (*mt_test_thread_init)(struct mt_test_cli *args,
		void *thread_args);
typedef int (*mt_test_thread_fini)(void *thread_args);
typedef void *(*mt_test_thread_func)(void *arg);

struct mt_test {
	/* size of data to be maintained for each of the threads */
	size_t thread_args_size;

	/* a function called for each of threads before spawning it */
	mt_test_thread_init thread_init_func;

	/* a function called for each of threads after its termination */
	mt_test_thread_fini thread_fini_func;

	/* a thread main function */
	mt_test_thread_func thread_func;
};

#endif /* MT_TEST */
