// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2021, Intel Corporation */

/*
 * rpma_mr_get_ptr.c -- rpma_mr_get_ptr multithreaded test
 */

#include <stdlib.h>
#include <librpma.h>

#include "mtt.h"

struct prestate {
	char *addr;
	struct rpma_peer *peer;
	void *mr_ptr;
	struct rpma_mr_local *mr;
};

/*
 * prestate_init -- obtain an ibv_context for a local IP address,
 * create a new peer object, allocate memory region, register the memory
 * and get the memory region pointer
 */
static void
prestate_init(void *prestate, struct mtt_result *tr)
{
	struct prestate *pr = (struct prestate *)prestate;
	struct ibv_context *dev;
	void *mr_ptr_exp;
	int ret;

	if ((ret = rpma_utils_get_ibv_context(pr->addr,
			RPMA_UTIL_IBV_CONTEXT_LOCAL, &dev))) {
		MTT_RPMA_ERR(tr, "rpma_utils_get_ibv_context", ret);
		return;
	}

	if ((ret = rpma_peer_new(dev, &pr->peer))) {
		MTT_RPMA_ERR(tr, "rpma_peer_new", ret);
		return;
	}

	pr->mr_ptr = mtt_malloc_aligned(KILOBYTE, tr);
	if (pr->mr_ptr == NULL)
		goto err_peer_delete;

	if ((ret = rpma_mr_reg(pr->peer, pr->mr_ptr, KILOBYTE,
			RPMA_MR_USAGE_READ_SRC, &pr->mr))) {
		MTT_RPMA_ERR(tr, "rpma_mr_reg", ret);
		goto err_free;
	}

	if ((ret = rpma_mr_get_ptr(pr->mr, &mr_ptr_exp))) {
		MTT_RPMA_ERR(tr, "rpma_mr_get_ptr", ret);
		goto err_mr_dereg;
	}

	if (mr_ptr_exp != pr->mr_ptr) {
		MTT_ERR_MSG(tr, "mr_ptr_exp != pr->mr_ptr", -1);
		goto err_mr_dereg;
	}

	return;

err_mr_dereg:
	(void) rpma_mr_dereg(&pr->mr);
err_free:
	free(pr->mr_ptr);
err_peer_delete:
	(void) rpma_peer_delete(&pr->peer);
}

/*
 * thread -- get the memory region pointer
 */
static void
thread(unsigned id, void *prestate, void *state,
		struct mtt_result *result)
{
	struct prestate *pr = (struct prestate *)prestate;
	void *mr_ptr;

	int ret = rpma_mr_get_ptr(pr->mr, &mr_ptr);
	if (ret) {
		MTT_RPMA_ERR(result, "rpma_mr_get_ptr", ret);
		return;
	}

	if (mr_ptr != pr->mr_ptr)
		MTT_ERR(result, "mr_ptr != pr->mr_ptr", EINVAL);
}

/*
 * prestate_fini -- deregister the memory region, free it
 * and delete the peer object
 */
static void
prestate_fini(void *prestate, struct mtt_result *tr)
{
	struct prestate *pr = (struct prestate *)prestate;
	int ret;

	if ((ret = rpma_mr_dereg(&pr->mr)))
		MTT_RPMA_ERR(tr, "rpma_mr_dereg", ret);

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

	struct prestate prestate = {args.addr, NULL};

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
