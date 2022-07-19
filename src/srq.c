// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2022, Fujitsu */

/*
 * srq.c -- librpma shared-RQ-related implementations
 */

#include <stdlib.h>
#include <string.h>

#include "cq.h"
#include "debug.h"
#include "librpma.h"
#include "log_internal.h"
#include "peer.h"
#include "mr.h"
#include "srq_cfg.h"

#ifdef TEST_MOCK_ALLOC
#include "cmocka_alloc.h"
#endif

struct rpma_srq {
	struct ibv_srq *ibv_srq;
	struct rpma_cq *rcq;
};

/* public librpma API */

/*
 * rpma_srq_new -- create a new shared RQ and a new shared receive CQ
 */
int
rpma_srq_new(struct rpma_peer *peer, struct rpma_srq_cfg *cfg,
		struct rpma_srq **srq_ptr)
{
	RPMA_DEBUG_TRACE;
	RPMA_FAULT_INJECTION(RPMA_E_INVAL, {});

	if (peer == NULL || srq_ptr == NULL)
		return RPMA_E_INVAL;

	if (cfg == NULL)
		cfg = rpma_srq_cfg_default();

	/* read shared RQ size from the configuration */
	uint32_t rq_size;
	(void) rpma_srq_cfg_get_rq_size(cfg, &rq_size);

	struct ibv_pd *pd = rpma_peer_get_ibv_pd(peer);

	struct ibv_srq *ibv_srq = NULL;
	struct ibv_srq_init_attr srq_init_attr;
	srq_init_attr.srq_context = NULL;
	srq_init_attr.attr.max_wr = rq_size;
	srq_init_attr.attr.max_sge = 1;
	srq_init_attr.attr.srq_limit = 0;

	RPMA_FAULT_INJECTION(RPMA_E_PROVIDER, {});
	ibv_srq = ibv_create_srq(pd, &srq_init_attr);
	if (ibv_srq == NULL) {
		RPMA_LOG_ERROR_WITH_ERRNO(errno, "ibv_create_srq()");
		return RPMA_E_PROVIDER;
	}

	/* read shared receive CQ size from the configuration */
	int rcqe;
	(void) rpma_srq_cfg_get_rcqe(cfg, &rcqe);

	int ret = 0;
	struct rpma_cq *rcq = NULL;
	if (rcqe) {
		ret = rpma_cq_new(pd->context, rcqe, NULL, &rcq);
		if (ret)
			goto err_srq_delete;
	}

	RPMA_FAULT_INJECTION_GOTO(RPMA_E_NOMEM, err_rpma_rcq_delete);
	*srq_ptr = (struct rpma_srq *)malloc(sizeof(struct rpma_srq));
	if (*srq_ptr == NULL) {
		ret = RPMA_E_NOMEM;
		goto err_rpma_rcq_delete;
	}

	(*srq_ptr)->ibv_srq = ibv_srq;
	(*srq_ptr)->rcq = rcq;

	return 0;

err_rpma_rcq_delete:
	(void) rpma_cq_delete(&rcq);

err_srq_delete:
	(void) ibv_destroy_srq(ibv_srq);

	return ret;
}

/*
 * rpma_srq_delete -- delete the shared RQ and the shared receive CQ
 */
int
rpma_srq_delete(struct rpma_srq **srq_ptr)
{
	RPMA_DEBUG_TRACE;
	RPMA_FAULT_INJECTION(RPMA_E_INVAL, {});

	if (srq_ptr == NULL)
		return RPMA_E_INVAL;

	struct rpma_srq *srq = *srq_ptr;
	int ret = 0;

	if (srq == NULL)
		return ret;

	ret = rpma_cq_delete(&srq->rcq);

	if (srq->ibv_srq) {
		RPMA_FAULT_INJECTION(RPMA_E_PROVIDER, {});
		errno = ibv_destroy_srq(srq->ibv_srq);
		if (!ret && errno) {
			RPMA_LOG_ERROR_WITH_ERRNO(errno, "ibv_destroy_srq()");
			ret = RPMA_E_PROVIDER;
		}
	}

	free(srq);
	*srq_ptr = NULL;

	return ret;
}

/*
 * rpma_srq_recv -- initiate the receive operation in shared RQ
 */
int
rpma_srq_recv(struct rpma_srq *srq, struct rpma_mr_local *dst,
		size_t offset, size_t len, const void *op_context)
{
	RPMA_DEBUG_TRACE;
	RPMA_FAULT_INJECTION(RPMA_E_INVAL, {});

	if (srq == NULL || (dst == NULL && (offset != 0 || len != 0)))
		return RPMA_E_INVAL;

	return rpma_mr_srq_recv(srq->ibv_srq, dst, offset, len, op_context);
}

/*
 * rpma_srq_get_rcq -- get the receive CQ from the shared RQ object
 */

int
rpma_srq_get_rcq(const struct rpma_srq *srq, struct rpma_cq **rcq_ptr)
{
	RPMA_DEBUG_TRACE;
	RPMA_FAULT_INJECTION(RPMA_E_INVAL, {});

	if (srq == NULL || rcq_ptr == NULL)
		return RPMA_E_INVAL;

	*rcq_ptr = srq->rcq;

	return 0;
}
