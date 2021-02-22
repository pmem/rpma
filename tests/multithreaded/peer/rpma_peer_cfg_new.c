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
};

static const char *api_name = "rpma_peer_cfg_new";

static void *thread_main(void *arg)
{
	int ret;

	/* RPMA resources */
	struct rpma_peer_cfg *pcfg = NULL;

	/* parameters */
	struct thread_args *p_thread_args = (struct thread_args *)arg;

	/* Create a remote peer's configuration structure */
	ret = rpma_peer_cfg_new(&pcfg);
	if (ret) {
		fprintf(stderr, "[thread #%d] %s failed: %s\n",
			p_thread_args->thread_num, api_name,
			rpma_err_2str(ret));
		exit(-1);
	}

	ret = rpma_peer_cfg_delete(&pcfg);
	if (ret) {
		fprintf(stderr, "[thread #%d] rpma_peer_cfg_delete \
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
	int ret = 0;
	int i;
	int thread_num = (int)strtoul(argv[1], NULL, 10);

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

	for (i = 0; i < thread_num; i++) {
		threads_args[i].thread_num = i;
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

	free(threads_args);

err_free_p_threads:
	free(p_threads);

	return ret;
}
