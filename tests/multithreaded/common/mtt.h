/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright 2021-2022, Intel Corporation */

/*
 * mtt.h -- a multithreaded tests' API
 *
 * For an example of how to use this API please see already existing
 * multithreaded tests especially the example test.
 */

#ifndef MTT
#define MTT

#include <stddef.h>
#include <stdbool.h>

#define KILOBYTE 1024

/* arguments coming from the command line */
struct mtt_args {
	unsigned threads_num;
	char *addr;
	unsigned port;
};

int mtt_parse_args(int argc, char *argv[], struct mtt_args *args);

#define MTT_PORT_STR_MAX 6
#define MTT_PORT_STR _mtt_port_str
#define MTT_PORT_INIT char _mtt_port_str[MTT_PORT_STR_MAX]

#define MTT_PORT_SET(base_port, thread_id) \
    snprintf(MTT_PORT_STR, MTT_PORT_STR_MAX, "%u", (base_port) + (thread_id))

#define MTT_ERRMSG_MAX 512

/* a store for any thread error message and the return value */
struct mtt_result {
	int ret;
	char errmsg[MTT_ERRMSG_MAX];
};

/*
 * mtt_base_file_name -- extract the exact file name from a file name with path
 */
const char *mtt_base_file_name(const char *file_name);

void *mtt_malloc_aligned(size_t size, struct mtt_result *tr);

/* on child's error print the error message to stderr */
#define CHILD_ERR_MSG(child_name, msg) \
	do { \
		fprintf(stderr, "%s %s:%d %s() -> %s\n", \
			child_name, mtt_base_file_name(__FILE__), \
			__LINE__, __func__, msg); \
	} while (0)

#define CHILD_ERR(child_name, func, msg) \
	do { \
		fprintf(stderr, "%s %s:%d %s() -> %s() failed: %s\n", \
			child_name, mtt_base_file_name(__FILE__), \
			__LINE__, __func__, func, msg); \
	} while (0)

#define CHILD_RPMA_ERR(child_name, func, err) \
	CHILD_ERR(child_name, func, rpma_err_2str(err))

#define SERVER_ERR_MSG(msg)		CHILD_ERR_MSG("[SERVER]", msg)
#define SERVER_RPMA_ERR(func, err)	CHILD_RPMA_ERR("[SERVER]", func, err)

/* on error populate the result and the error message */
#define MTT_ERR_MSG(result, msg, err, ...) \
	do { \
		if ((result) == NULL) \
			break; \
		char msg_buf[MTT_ERRMSG_MAX / 2]; \
		snprintf(msg_buf, MTT_ERRMSG_MAX / 2 - 1, \
			msg, ##__VA_ARGS__); \
		(result)->ret = err; \
		snprintf((result)->errmsg, MTT_ERRMSG_MAX - 1, \
			"%s:%d %s() -> %s\n", \
			mtt_base_file_name(__FILE__), __LINE__, __func__, \
			msg_buf); \
	} while (0)

/* on error populate the result and the error string */
#define MTT_ERR(result, func, err) \
	do { \
		if ((result) == NULL) \
			break; \
		(result)->ret = err; \
		snprintf((result)->errmsg, MTT_ERRMSG_MAX - 1, \
			"%s:%d %s() -> %s() failed: %s\n", \
			mtt_base_file_name(__FILE__), __LINE__, __func__, \
			func, strerror(err)); \
	} while (0)

/* on librpma error populate the result and the error string */
#define MTT_RPMA_ERR(result, func, err) \
	do { \
		if ((result) == NULL) \
			break; \
		(result)->ret = err; \
		snprintf((result)->errmsg, MTT_ERRMSG_MAX - 1, \
			"%s:%d %s() -> %s() failed: %s\n", \
			mtt_base_file_name(__FILE__), __LINE__, __func__, \
			func, rpma_err_2str(err)); \
	} while (0)

/*
 * mtt_prestate_init_fini_func -- a function type used for initialization and
 * cleanup of prestate. Run once.
 *
 * Arguments:
 * - prestate  - a pointer to the test-provided data. It is the only function
 *               type in which the prestate is expected to be modified.
 * - result    - the result. On error the test is responsible for providing
 *               the error details (using e.g. MTT_ERR or MTT_RPMA_ERR macros).
 */
typedef void (*mtt_prestate_init_fini_func)(void *prestate,
		struct mtt_result *result);

/*
 * mtt_thread_init_fini_func -- a function type used for all initialization and
 * cleanup steps
 *
 * Arguments:
 * - id        - a thread identifier. It is constant for the whole life of
 *               the thread including sequential initialization and sequential
 *               cleanup.
 * - prestate  - a pointer to the test-provided data passed to all threads in
 *               all steps. It is shared in a non-thread-safe way.
 * - state_ptr - a pointer to thread-related data. The test can allocate and
 *               store their specific data here at any point. Accessing it is
 *               always thread-safe. Once the data is stored the test is also
 *               responsible for freeing it.
 * - result    - the result. On error the test is responsible for providing
 *               the error details (using e.g. MTT_ERR or MTT_RPMA_ERR macros),
 *               the test should not print anything to stdout nor stderr during
 *               parallel steps of the test (thread_init_func, thread_func,
 *               and thread_fini_func).
 */
typedef void (*mtt_thread_init_fini_func)(unsigned id, void *prestate,
		void **state_ptr, struct mtt_result *result);

/*
 * mtt_thread_func -- a function type used for the main execution step
 *
 * Arguments:
 * - id       - a thread identifier. It is constant for the whole life of
 *              the thread including sequential initialization and sequential
 *              cleanup.
 * - prestate - a pointer to the test-provided data passed to all threads in all
 *              steps. It is shared in a non-thread-safe way.
 * - state    - a pointer to thread-related data. At this point, it is available
 *              as long as it was prepared during one of the initialization
 *              steps. Note it should not be freed during this step. For tips
 *              on how to allocate/free the thread-related data please see
 *              mtt_thread_init_fini_func.
 * - result   - the result. On error the test is responsible for providing
 *              the error details (using e.g. MTT_ERR or MTT_RPMA_ERR macros),
 *              the test should not print anything to stdout nor stderr during
 *              parallel steps of the test (thread_init_func, thread_func,
 *              and thread_fini_func).
 */
typedef void (*mtt_thread_func)(unsigned id, void *prestate, void *state,
		struct mtt_result *result);

/*
 * mtt_child_process_func -- a function type used for the child process
 *                           e.g. as another side of the connection
 *
 * Arguments:
 * - prestate - a pointer to the test-provided data.
 */
typedef int (*mtt_child_process_func)(void *prestate);

/*
 * mtt_start_child - define a time when the child process is started
 */
enum mtt_start_child {
	MTT_START_CHILD_BEFORE_PRESTATE_INIT_FUNC,
	MTT_START_CHILD_BEFORE_THREAD_SEQ_INIT_FUNC,
	MTT_START_CHILD_BEFORE_THREAD_INIT_FUNC,
	MTT_START_CHILD_BEFORE_THREAD_FUNC,
	MTT_START_CHILD_BEFORE_JOINING_THREADS
};

struct mtt_test {
	/*
	 * a pointer to test-provided data passed on all initialization steps
	 * (both sequential and parallel) and also on thread_func
	 */
	void *prestate;

	/*
	 * A function called only once before the sequential initialization of
	 * all threads. It is dedicated to initializing the prestate.
	 */
	mtt_prestate_init_fini_func prestate_init_func;

	/*
	 * a function called for each of threads before spawning it (sequential)
	 */
	mtt_thread_init_fini_func thread_seq_init_func;

	/*
	 * a function called at the beginning of each thread
	 * (parallel but before synchronizing all threads)
	 */
	mtt_thread_init_fini_func thread_init_func;

	/*
	 * a thread main function (parallel and after synchronizing all threads)
	 */
	mtt_thread_func thread_func;

	/* a function called at the end of each thread (parallel) */
	mtt_thread_init_fini_func thread_fini_func;

	/*
	 * a function called for each of threads after its termination
	 * (sequential)
	 */
	mtt_thread_init_fini_func thread_seq_fini_func;

	/*
	 * A function called only once after the sequential clean up of all
	 * threads. It is dedicated to cleaning up the prestate.
	 */
	mtt_prestate_init_fini_func prestate_fini_func;

	/*
	 * A function of the child process.
	 * If it is not NULL, the child process is started
	 * using fork() at the very beginning of the test.
	 */
	mtt_child_process_func child_process_func;

	/*
	 * A pointer to test-provided data passed to the child process function.
	 */
	void *child_prestate;

	/*
	 * Set a time when the child process should be started.
	 */
	enum mtt_start_child child_start;
};

int mtt_run(struct mtt_test *test, unsigned threads_num);

#endif /* MTT */
