// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2021, Fujitsu */

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
 * rpma_cq_get_fd -- get a file descriptor of the completion event channel
 * from the rpma_cq object
 *
 * XXX this function does not follow the rpma_cq_get_ibv_cq() schema
 * because it is planned to be made public in the near future so it
 * will have to handle cq and fd validation on its own.
 *
 * ASSUMPTIONS
 * - cq != NULL && fd != NULL
 */
int
rpma_cq_get_fd(const struct rpma_cq *cq, int *fd)
{
	*fd = cq->channel->fd;

	return 0;
}

/*
 * rpma_cq_wait -- wait for a completion event from the rpma_cq object
 * and ack the completion event
 *
 * ASSUMPTIONS
 * - cq != NULL
 */
int
rpma_cq_wait(struct rpma_cq *cq)
{
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
 * rpma_cq_get_completion -- receive an operation completion from
 * the rpma_cq object
 *
 * ASSUMPTIONS
 * - cq != NULL && cmpl != NULL
 */
int
rpma_cq_get_completion(struct rpma_cq *cq, struct rpma_completion *cmpl)
{
	struct ibv_wc wc = {0};
	int result = ibv_poll_cq(cq->cq, 1 /* num_entries */, &wc);
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
	} else if (result > 1) {
		RPMA_LOG_ERROR(
			"ibv_poll_cq() returned %d where 0 or 1 is expected",
			result);
		return RPMA_E_UNKNOWN;
	}

	switch (wc.opcode) {
	case IBV_WC_RDMA_READ:
		cmpl->op = RPMA_OP_READ;
		break;
	case IBV_WC_RDMA_WRITE:
		cmpl->op = RPMA_OP_WRITE;
		break;
	case IBV_WC_SEND:
		cmpl->op = RPMA_OP_SEND;
		break;
	case IBV_WC_RECV:
		cmpl->op = RPMA_OP_RECV;
		break;
	case IBV_WC_RECV_RDMA_WITH_IMM:
		cmpl->op = RPMA_OP_RECV_RDMA_WITH_IMM;
		break;
	default:
		RPMA_LOG_ERROR("unsupported wc.opcode == %d", wc.opcode);
		return RPMA_E_NOSUPP;
	}

	cmpl->op_context = (void *)wc.wr_id;
	cmpl->byte_len = wc.byte_len;
	cmpl->op_status = wc.status;
	/* 'wc_flags' is of 'int' type in older versions of libibverbs */
	cmpl->flags = (unsigned)wc.wc_flags;

	/*
	 * The value of imm_data can only be placed in the receive Completion
	 * Queue Element.
	 */
	if ((cmpl->op == RPMA_OP_RECV) ||
			(cmpl->op == RPMA_OP_RECV_RDMA_WITH_IMM)) {
		if (cmpl->flags & IBV_WC_WITH_IMM)
			cmpl->imm = ntohl(wc.imm_data);
	}

	if (unlikely(wc.status != IBV_WC_SUCCESS)) {
		RPMA_LOG_WARNING("failed rpma_completion(op_context=0x%" PRIx64
				", op_status=%s)",
				cmpl->op_context,
				ibv_wc_status_str(cmpl->op_status));
	}

	return 0;
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
