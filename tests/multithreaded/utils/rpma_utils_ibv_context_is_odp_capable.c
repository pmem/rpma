// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2021, Intel Corporation */

/*
 * rpma_utils_ibv_context_is_odp_capable.c -- 'check odp' multithreaded test
 */

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>

#include <librpma.h>

struct thread_args {
	int thread_num;
	struct ibv_context *dev;
};

static const char *api_name = "rpma_utils_ibv_context_is_odp_capable";

static void *thread_main(void *arg)
{
	int ret;
	int is_odp_supported = 0;

	/* parameters */
	struct thread_args *p_thread_args = (struct thread_args *)arg;

	/* check if the device context's capability supports On-Demand Paging */
	ret = rpma_utils_ibv_context_is_odp_capable(p_thread_args->dev,
							&is_odp_supported);

	if (ret) {
		fprintf(stderr, "[thread #%d] %s failed: %s\n",
			p_thread_args->thread_num, api_name,
			rpma_err_2str(ret));
		exit(-1);
	}

	return NULL;
}

int
main(int argc, char *argv[])
{
	if (argc < 3) {
		fprintf(stderr, "usage: %s <thread_num> <addr>\n", argv[0]);
		exit(-1);
	}

	/* configure logging thresholds to see more details */
	rpma_log_set_threshold(RPMA_LOG_THRESHOLD, RPMA_LOG_LEVEL_INFO);
	rpma_log_set_threshold(RPMA_LOG_THRESHOLD_AUX, RPMA_LOG_LEVEL_INFO);

	/* parameters */
	int ret = 0;
	int i;
	int thread_num = (int)strtoul(argv[1], NULL, 10);
	char *addr = argv[2];

	/* resources */
	struct ibv_context *dev = NULL;
	/* obtain an IBV context for a local IP address */
	ret = rpma_utils_get_ibv_context(addr, RPMA_UTIL_IBV_CONTEXT_LOCAL,
									&dev);
	if (ret) {
		fprintf(stderr, "get ibv context failed");
		exit(-1);
	}

	pthread_t *p_threads;
	p_threads = malloc(sizeof(pthread_t) * (unsigned int)thread_num);
	if (p_threads == NULL) {
		fprintf(stderr, "malloc failed");
		exit(-1);
	}

	struct thread_args *threads_args = malloc(sizeof(struct thread_args)
						* (unsigned int)thread_num);
	if (threads_args == NULL) {
		fprintf(stderr, "malloc failed");
		exit(-1);
	}

	for (i = 0; i < thread_num; i++) {
		threads_args[i].thread_num = i;
		threads_args[i].dev = dev;
		if ((ret = pthread_create(&p_threads[i], NULL, thread_main,
				&threads_args[i])) != 0) {
			fprintf(stderr, "Cannot start a thread #%d: %s\n",
				i, strerror(ret));
			return ret;
		}
	}

	for (i = thread_num - 1; i >= 0; i--)
		pthread_join(p_threads[i], NULL);

	return ret;
}
