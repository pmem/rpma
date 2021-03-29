// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2021, Intel Corporation */

/*
 * rpma_peer_cfg_from_descriptor.c -- multithreaded test
 */

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>

#include <librpma.h>

#define DIRECT_WRITE_TO_PMEM true

struct thread_args {
	int thread_num;
	const void *desc;
	size_t desc_size;
};

static const char *api_name = "rpma_peer_cfg_from_descriptor";

static void *thread_main(void *arg)
{
	int ret;
	bool direct_write_to_pmem = false;

	/* RPMA resources */
	struct rpma_peer_cfg *pcfg = NULL;

	/* parameters */
	struct thread_args *p_thread_args = (struct thread_args *)arg;

	/* Create a remote peer configuration structure from input descriptor */
	ret = rpma_peer_cfg_from_descriptor(p_thread_args->desc,
			p_thread_args->desc_size, &pcfg);
	if (ret) {
		fprintf(stderr, "[thread #%d] %s failed: %s\n",
			p_thread_args->thread_num, api_name,
			rpma_err_2str(ret));
		exit(-1);
	}

	/* check if direct write to PMEM is expected(supported or not) */
	ret = rpma_peer_cfg_get_direct_write_to_pmem(pcfg,
			&direct_write_to_pmem);
	if (ret || direct_write_to_pmem != DIRECT_WRITE_TO_PMEM) {
		fprintf(stderr, "[thread #%d] \
			rpma_peer_cfg_get_direct_write_to_pmem failed: %s, \
			direct_write_to_pmem = %d\n", p_thread_args->thread_num,
			rpma_err_2str(ret), direct_write_to_pmem);
		goto err_peer_cfg_delete;
	}

err_peer_cfg_delete:
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
		fprintf(stderr, "calloc() failed");
		return -1;
	}

	struct thread_args *threads_args = calloc((size_t)thread_num,
				sizeof(struct thread_args));
	bool direct_write_to_pmem = DIRECT_WRITE_TO_PMEM;
	size_t desc_size = sizeof(bool);

	if (threads_args == NULL) {
		fprintf(stderr, "calloc() failed");
		ret = -1;
		goto err_free_p_threads;
	}

	for (i = 0; i < thread_num; i++) {
		threads_args[i].thread_num = i;
		threads_args[i].desc = (uint8_t *)(&direct_write_to_pmem);
		threads_args[i].desc_size = desc_size;
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
