// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2021, Intel Corporation */

/*
 * rpma_ep_get_fd.c -- rpma_ep_get_fd multithreaded test
 */

#include <stdlib.h>
#include <librpma.h>

#include "mtt.h"

struct prestate {
	char *addr;
	unsigned port;
	struct ibv_context *dev;
	struct rpma_peer *peer;
	struct rpma_ep *ep;
	int ep_fd_exp;
};

/*
 * prestate_init -- obtain an ibv_context for a local IP address,
 * create a new peer object, start a listening endpoint and
 * get the endpoint's event file descriptor
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

	MTT_PORT_INIT;
	MTT_PORT_SET(pr->port, 0);
	if ((ret = rpma_ep_listen(pr->peer, pr->addr, MTT_PORT_STR, &pr->ep))) {
		MTT_RPMA_ERR(tr, "rpma_ep_listen", ret);
		(void) rpma_peer_delete(&pr->peer);
		return;
	}

	if ((ret = rpma_ep_get_fd(pr->ep, &pr->ep_fd_exp))) {
		MTT_RPMA_ERR(tr, "rpma_ep_get_fd", ret);
		(void) rpma_ep_shutdown(&pr->ep);
		(void) rpma_peer_delete(&pr->peer);
	}
}

/*
 * thread -- get the endpoint's event file descriptor
 */
static void
thread(unsigned id, void *prestate, void *state,
		struct mtt_result *result)
{
	struct prestate *pr = (struct prestate *)prestate;
	int ep_fd;

	int ret = rpma_ep_get_fd(pr->ep, &ep_fd);
	if (ret) {
		MTT_RPMA_ERR(result, "rpma_ep_get_fd", ret);
		return;
	}

	if (ep_fd != pr->ep_fd_exp)
		MTT_ERR(result, "rpma_ep_get_fd returned an unexpected value",
				EINVAL);
}

/*
 * prestate_fini -- shutdown the endpoint and delete the peer object
 */
static void
prestate_fini(void *prestate, struct mtt_result *tr)
{
	struct prestate *pr = (struct prestate *)prestate;
	int ret;

	if ((ret = rpma_ep_shutdown(&pr->ep)))
		MTT_RPMA_ERR(tr, "rpma_ep_shutdown", ret);

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
