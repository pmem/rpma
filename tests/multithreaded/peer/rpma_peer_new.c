// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2021, Intel Corporation */

/*
 * rpma_peer_cfg_new.c -- multithreaded test
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
	int is_odp_supported_exp;
};

static const char *api_name = "rpma_peer_new";

static void *thread_main(void *arg)
{
	int ret;

	/* RPMA resources */
	struct rpma_peer *peer = NULL;

	/* parameters */
	struct thread_args *p_thread_args = (struct thread_args *)arg;

	/* create a new peer object */
	ret = rpma_peer_new(p_thread_args->dev, &peer);

	if (ret) {
		fprintf(stderr, "[thread #%d] %s failed: %s\n",
			p_thread_args->thread_num, api_name,
			rpma_err_2str(ret));
		exit(-1);
	}

	/* delete the peer object */
	ret = rpma_peer_delete(&peer);
	if (ret) {
		fprintf(stderr, "[thread #%d] rpma_peer_delete \
			failed: %s\n", p_thread_args->thread_num,
			rpma_err_2str(ret));
		exit(-1);
	}

	return NULL;
}

int
main(int argc, char *argv[])
{
	if (argc < 2) {
		fprintf(stderr, "usage: %s <thread_num>\n", argv[0]);
		return -1;
	}

	/* configure logging thresholds to see more details */
	rpma_log_set_threshold(RPMA_LOG_THRESHOLD, RPMA_LOG_LEVEL_INFO);
	rpma_log_set_threshold(RPMA_LOG_THRESHOLD_AUX, RPMA_LOG_LEVEL_INFO);

	/* parameters */
	int i, ret = 0;
	int is_odp_supported = 0;
	int thread_num = (int)strtoul(argv[1], NULL, 10);
	char *addr = argv[2];
	char *port = argv[3];

	pthread_t *p_threads;
	p_threads = calloc((size_t)thread_num, sizeof(pthread_t));
	if (p_threads == NULL) {
		fprintf(stderr, "malloc() failed");
		return -1;
	}

	struct thread_args *threads_args = calloc((size_t)thread_num,
				sizeof(struct thread_args));
	if (threads_args == NULL) {
		fprintf(stderr, "malloc() failed");
		ret = -1;
		goto err_free_p_threads;
	}

	/* obtain an IBV context for a remote IP address */
	struct ibv_context *dev;
	ret = rpma_utils_get_ibv_context(addr, RPMA_UTIL_IBV_CONTEXT_REMOTE,
			&dev);
	if (ret)
		goto err_free_threads_args;

	/* check if the device context's capability supports On-Demand Paging */
	ret = rpma_utils_ibv_context_is_odp_capable(dev,
				&is_odp_supported);
	if (ret)
		goto err_free_threads_args;


	for (i = 0; i < thread_num; i++) {
		threads_args[i].thread_num = i;
		threads_args[i].dev = dev;
		threads_args[i].is_odp_supported_exp = is_odp_supported;
	}

	for (i = 0; i < thread_num; i++) {
		if ((ret = pthread_create(&p_threads[i], NULL, thread_main,
				&threads_args[i])) != 0) {
			fprintf(stderr, "Cannot start the thread #%d: %s\n",
				i, strerror(ret));
			/*
			 * Set thread_num to the number of already created
			 * threads to join them below.
			 */
			thread_num = i;
			/* return -1 on error */
			ret = -1;
			break;
		}
	}

	for (i = thread_num - 1; i >= 0; i--)
		pthread_join(p_threads[i], NULL);

err_free_threads_args:
	free(threads_args);

err_free_p_threads:
	free(p_threads);

	return ret;
}
