// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2021, Intel Corporation */

/*
 * rpma_mr_get_descriptor.c -- rpma_mr_get_descriptor multithreaded test
 */

#include <stdlib.h>
#include <librpma.h>

#include "mtt.h"

struct prestate {
	char *addr;
	struct rpma_peer *peer;
	void *mr_ptr;
	struct rpma_mr_local *mr;
	size_t mr_desc_size_exp;
	char descriptor_exp[KILOBYTE];
};

/*
 * prestate_init -- obtain an ibv_context for a local IP address,
 * create a new peer object, allocate and register memory region,
 * and prepare an expected descriptor value for the memory region.
 */
static void
prestate_init(void *prestate, struct mtt_result *tr)
{
	struct prestate *pr = (struct prestate *)prestate;
	struct ibv_context *ibv_ctx;
	int ret;

	if ((ret = rpma_utils_get_ibv_context(pr->addr,
			RPMA_UTIL_IBV_CONTEXT_LOCAL, &ibv_ctx))) {
		MTT_RPMA_ERR(tr, "rpma_utils_get_ibv_context", ret);
		return;
	}

	if ((ret = rpma_peer_new(ibv_ctx, &pr->peer))) {
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

	if ((ret = rpma_mr_get_descriptor_size(pr->mr,
			&pr->mr_desc_size_exp))) {
		MTT_RPMA_ERR(tr, "rpma_mr_get_descriptor_size", ret);
		goto err_mr_dereg;
	}

	if (pr->mr_desc_size_exp > KILOBYTE) {
		MTT_ERR_MSG(tr, "mr_desc_size_exp > KILOBYTE", -1);
		goto err_mr_dereg;
	}

	if ((ret = rpma_mr_get_descriptor(pr->mr, &pr->descriptor_exp[0]))) {
		MTT_RPMA_ERR(tr, "rpma_mr_get_descriptor", ret);
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
 * thread -- get the memory region's descriptor and validate it
 */
static void
thread(unsigned id, void *prestate, void *state,
		struct mtt_result *result)
{
	struct prestate *pr = (struct prestate *)prestate;
	size_t mr_desc_size;
	char descriptor[KILOBYTE];
	int ret;

	if ((ret = rpma_mr_get_descriptor_size(pr->mr, &mr_desc_size))) {
		MTT_RPMA_ERR(result, "rpma_mr_get_descriptor_size", ret);
		return;
	}

	if (mr_desc_size != pr->mr_desc_size_exp) {
		MTT_ERR_MSG(result, "mr_desc_size != pr->mr_desc_size_exp", -1);
		return;
	}

	if ((ret = rpma_mr_get_descriptor(pr->mr, &descriptor[0]))) {
		MTT_RPMA_ERR(result, "rpma_mr_get_descriptor", ret);
		return;
	}

	if ((ret = memcmp(descriptor, pr->descriptor_exp, mr_desc_size)))
		MTT_ERR_MSG(result, "descriptor != pr->descriptor_exp", -1);
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

	struct prestate prestate = {args.addr};

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
