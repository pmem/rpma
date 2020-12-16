// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020, Intel Corporation */

/*
 * 01-rpma_utils_get_ibv_context.c -- this API's multithreaded test
 */

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>

#include <librpma.h>

typedef struct mtt_args {
	int ntd;
	char *addr;
} mtt_args;

const char *api_name = "rpma_utils_get_ibv_context";

static void *con_thread(void *arg)
{
	int ret = 0;
	/* parameters */
	mtt_args *p_mttargs = (mtt_args *)arg;
	int ntd = p_mttargs->ntd;
	char *addr = p_mttargs->addr;

	/* resources */
	struct ibv_context *dev = NULL;

	/* obtain an IBV context for a local IP address */
	ret = rpma_utils_get_ibv_context(addr, RPMA_UTIL_IBV_CONTEXT_LOCAL,
			&dev);
	if (ret) {
		printf("-ERR thread #%d failed at %s: ret = %d\n",
				ntd, api_name, ret);
		abort();
	} else {
		printf("-OK thread #%d succeeded at %s!\n", ntd, api_name);
	}

	return NULL;
}

int
main(int argc, char *argv[])
{
	if (argc < 3) {
		fprintf(stderr, "usage: %s <thread_nums> <addr>\n", argv[0]);
		abort();
	}

	/* configure logging thresholds to see more details */
	rpma_log_set_threshold(RPMA_LOG_THRESHOLD, RPMA_LOG_LEVEL_INFO);
	rpma_log_set_threshold(RPMA_LOG_THRESHOLD_AUX, RPMA_LOG_LEVEL_INFO);

	/* parameters */
	int i;
	int tn = (int)strtoul(argv[1], NULL, 10);
	char *addr = argv[2];
	int ret = 0;

	pthread_t *p_tds = malloc(sizeof(pthread_t) * (long unsigned int)tn);
	mtt_args *p_args = malloc(sizeof(mtt_args) * (long unsigned int)tn);

	for (i = 0; i < tn; i++) {
		p_args[i].ntd = i;
		p_args[i].addr = addr;
		if ((ret = pthread_create(&p_tds[i], NULL, con_thread,
				&p_args[i])) != 0) {
			printf("-ERR Can't start thread #%d: ret = %d\n",
					i, ret);
			return ret;
		}
	}

	for (i = tn - 1; i >= 0; i--)
		pthread_join(p_tds[i], NULL);

	printf("-OK Pass multithreaded 01-%s!\n", api_name);
	return 0;
}
