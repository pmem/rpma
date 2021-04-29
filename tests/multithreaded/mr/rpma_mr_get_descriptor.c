// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2021, Intel Corporation */

/*
 * rpma_mr_get_descriptor.c -- rpma_mr_get_descriptor multithreaded test
 */

#include <stdlib.h>
#include <librpma.h>

#include "mtt.h"

#define KILOBYTE 1024
#define DESCRIPTORS_MAX_SIZE 24

struct prestate {
	char *addr;
	unsigned port;
	struct ibv_context *dev;
	struct rpma_peer *peer;
	void *mr_ptr;
	struct rpma_mr_local *mr;
};

struct state {
	void *mr_ptr;
	struct rpma_mr_local *mr;
};

/*
 * prestate_init -- obtain an ibv_context for a local IP address,
 * create a new peer object, allocate and register memory region
 */
static void
prestate_init(void *prestate, struct mtt_result *tr)
{
	struct prestate *pr = (struct prestate *)prestate;
	int ret;

	if ((ret = rpma_utils_get_ibv_context(pr->addr,
			RPMA_UTIL_IBV_CONTEXT_LOCAL, &pr->dev))) {
		MTT_RPMA_ERR(tr, "rpma_utils_get_ibv_context", ret);
		return;
	}

	if ((ret = rpma_peer_new(pr->dev, &pr->peer))) {
		MTT_RPMA_ERR(tr, "rpma_peer_new", ret);
		return;
	}

	pr->mr_ptr = mtt_malloc_aligned(KILOBYTE);
	if (pr->mr_ptr == NULL) {
		MTT_ERR(tr, "mtt_malloc_aligned", errno);
		return;
	}

	if ((ret = rpma_mr_reg(pr->peer, pr->mr_ptr, KILOBYTE,
			RPMA_MR_USAGE_READ_SRC, &pr->mr)))
		MTT_RPMA_ERR(tr, "rpma_mr_reg", ret);
}

/*
 * thread -- get the memory region's descriptor
 */
static void
thread(unsigned id, void *prestate, void *state,
		struct mtt_result *result)
{
	struct prestate *pr = (struct prestate *)prestate;
	int ret;

	size_t mr_desc_size;
	ret = rpma_mr_get_descriptor_size(pr->mr, &mr_desc_size);
	if ((ret = rpma_mr_get_descriptor_size(pr->mr, &mr_desc_size))) {
		MTT_RPMA_ERR(result, "rpma_mr_get_descriptor_size", ret);
		return;
	}

	char descriptors[DESCRIPTORS_MAX_SIZE];
	if ((ret = rpma_mr_get_descriptor(pr->mr, &descriptors[0])))
		MTT_RPMA_ERR(result, "rpma_mr_get_descriptor", ret);
}

/*
 * prestate_fini -- deregister and free the memory region,
 * and delete the peer object
 */
static void
prestate_fini(void *prestate, struct mtt_result *tr)
{
	struct prestate *pr = (struct prestate *)prestate;
	int ret;

	if (pr->mr) {
		if ((ret = rpma_mr_dereg(&pr->mr)))
			MTT_RPMA_ERR(tr, "rpma_mr_dereg", ret);
	}

	free(pr->mr_ptr);

	if ((ret = rpma_peer_delete(&pr->peer)))
		MTT_RPMA_ERR(tr, "rpma_peer_delete", ret);
}

int
main(int argc, char *argv[])
{
	struct mtt_args args = {0};

	if (mtt_parse_args(argc, argv, &args))
		return -1;

	struct prestate prestate = {args.addr, args.port, NULL, NULL};

	struct mtt_test test = {
			&prestate,
			prestate_init,
			NULL,
			NULL,
			thread,
			NULL,
			NULL,
			prestate_fini
	};

	return mtt_run(&test, args.threads_num);
}
