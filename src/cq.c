// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2022, Intel Corporation */
/* Copyright 2021-2022, Fujitsu */

/*
 * cq.c -- librpma completion-queue-related implementations
 */

#include <errno.h>
#include <inttypes.h>
#include <stdlib.h>
#include <arpa/inet.h>

#include "common.h"
#include "cq.h"
#include "log_internal.h"

#ifdef TEST_MOCK_ALLOC
#include "cmocka_alloc.h"
#endif

struct rpma_cq {
	struct ibv_comp_channel *channel; /* completion event channel */
	struct ibv_cq *cq; /* completion queue */
};

/* internal librpma API */

/*
 * rpma_cq_get_ibv_cq -- get the CQ member from the rpma_cq object
 *
 * ASSUMPTIONS
 * - cq != NULL
 */
struct ibv_cq *
rpma_cq_get_ibv_cq(const struct rpma_cq *cq)
{
	return cq->cq;
}

/*
 * rpma_cq_new -- create a completion channel and CQ and then
 * encapsulate them in a rpma_cq object
 *
 * ASSUMPTIONS
 * - dev != NULL && cq_ptr != NULL
 */
int
rpma_cq_new(struct ibv_context *dev, int cqe, struct rpma_cq **cq_ptr)
{
	int ret = 0;

	/* create a completion channel */
	struct ibv_comp_channel *channel = ibv_create_comp_channel(dev);
	if (channel == NULL) {
		RPMA_LOG_ERROR_WITH_ERRNO(errno, "ibv_create_comp_channel()");
		return RPMA_E_PROVIDER;
	}

	/* create a CQ */
	struct ibv_cq *cq = ibv_create_cq(dev, cqe,
				NULL /* cq_context */,
				channel /* channel */,
				0 /* comp_vector */);
	if (cq == NULL) {
		RPMA_LOG_ERROR_WITH_ERRNO(errno, "ibv_create_cq()");
		ret = RPMA_E_PROVIDER;
		goto err_destroy_comp_channel;
	}

	/* request for the next completion on the completion channel */
	errno = ibv_req_notify_cq(cq, 0 /* all completions */);
	if (errno) {
		RPMA_LOG_ERROR_WITH_ERRNO(errno, "ibv_req_notify_cq()");
		ret = RPMA_E_PROVIDER;
		goto err_destroy_cq;
	}

	*cq_ptr = (struct rpma_cq *)malloc(sizeof(struct rpma_cq));
	if (*cq_ptr == NULL) {
		ret = RPMA_E_NOMEM;
		goto err_destroy_cq;
	}

	(*cq_ptr)->channel = channel;
	(*cq_ptr)->cq = cq;

	return 0;

err_destroy_cq:
	(void) ibv_destroy_cq(cq);

err_destroy_comp_channel:
	(void) ibv_destroy_comp_channel(channel);

	return ret;
}

/*
 * rpma_cq_delete -- destroy the CQ and the completion channel and then
 * free the encapsulating rpma_cq object
 *
 * ASSUMPTIONS
 * - cq_ptr != NULL
 */
int
rpma_cq_delete(struct rpma_cq **cq_ptr)
{
	struct rpma_cq *cq = *cq_ptr;
	int ret = 0;

	/* it is possible for cq to be NULL (e.g. rcq) */
	if (cq == NULL)
		return ret;

	errno = ibv_destroy_cq(cq->cq);
	if (errno) {
		RPMA_LOG_ERROR_WITH_ERRNO(errno, "ibv_destroy_cq()");
		ret = RPMA_E_PROVIDER;
	}

	errno = ibv_destroy_comp_channel(cq->channel);
	if (!ret && errno) {
		RPMA_LOG_ERROR_WITH_ERRNO(errno, "ibv_destroy_comp_channel()");
		ret = RPMA_E_PROVIDER;
	}

	free(cq);
	*cq_ptr = NULL;

	return ret;
}

/* public librpma API */

/*
 * rpma_cq_get_fd -- get a file descriptor of the completion event channel
 * from the CQ
 */
int
rpma_cq_get_fd(const struct rpma_cq *cq, int *fd)
{
	if (cq == NULL || fd == NULL)
		return RPMA_E_INVAL;

	*fd = cq->channel->fd;

	return 0;
}

/*
 * rpma_cq_wait -- wait for a completion event from the CQ and ack
 * the completion event
 */
int
rpma_cq_wait(struct rpma_cq *cq)
{
	if (cq == NULL)
		return RPMA_E_INVAL;

	/* wait for the completion event */
	struct ibv_cq *ev_cq;	/* unused */
	void *ev_ctx;		/* unused */
	if (ibv_get_cq_event(cq->channel, &ev_cq, &ev_ctx))
		return RPMA_E_NO_COMPLETION;

	/*
	 * ACK the collected CQ event.
	 *
	 * XXX for performance reasons, it may be beneficial to ACK more than
	 * one CQ event at the same time.
	 */
	ibv_ack_cq_events(cq->cq, 1 /* # of CQ events */);

	/* request for the next event on the CQ channel */
	errno = ibv_req_notify_cq(cq->cq, 0 /* all completions */);
	if (errno) {
		RPMA_LOG_ERROR_WITH_ERRNO(errno, "ibv_req_notify_cq()");
		return RPMA_E_PROVIDER;
	}

	return 0;
}

/*
 * rpma_cq_get_wc -- receive one or more completions from the CQ
 */
int
rpma_cq_get_wc(struct rpma_cq *cq, int num_entries, struct ibv_wc *wc,
		int *num_entries_got)
{
	if (cq == NULL || num_entries < 1 || wc == NULL)
		return RPMA_E_INVAL;

	if (num_entries > 1 && num_entries_got == NULL)
		return RPMA_E_INVAL;

	int result = ibv_poll_cq(cq->cq, num_entries, wc);
	if (result == 0) {
		/*
		 * There may be an extra CQ event with no completion in the CQ.
		 */
		RPMA_LOG_DEBUG("No completion in the CQ");
		return RPMA_E_NO_COMPLETION;
	} else if (result < 0) {
		/* ibv_poll_cq() may return only -1; no errno provided */
		RPMA_LOG_ERROR("ibv_poll_cq() failed (no details available)");
		return RPMA_E_PROVIDER;
	} else if (result > num_entries) {
		RPMA_LOG_ERROR(
			"ibv_poll_cq() returned %d where <= %d is expected",
			result, num_entries);
		return RPMA_E_UNKNOWN;
	}

	if (num_entries_got)
		*num_entries_got = result;

	return 0;
}
