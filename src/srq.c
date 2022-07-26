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
#include "srq.h"

#ifdef TEST_MOCK_ALLOC
#include "cmocka_alloc.h"
#endif

struct rpma_srq {
	struct ibv_srq *ibv_srq;
	struct rpma_cq *rcq;
};

/* internal librpma API */

/*
 * rpma_srq_get_ibv_srq -- get the shared CQ member from the rpma_srq object
 *
 * ASSUMPTIONS
 * - srq != NULL
 */
struct ibv_srq *
rpma_srq_get_ibv_srq(const struct rpma_srq *srq)
{
	return srq->ibv_srq;
}

/* public librpma API */

/*
 * rpma_srq_new -- create a new shared RQ and a new shared receive CQ
 * if the size of the receive CQ in cfg is greater than 0
 */
int
rpma_srq_new(struct rpma_peer *peer, struct rpma_srq_cfg *cfg, struct rpma_srq **srq_ptr)
{
	RPMA_DEBUG_TRACE;
	RPMA_FAULT_INJECTION(RPMA_E_INVAL, {});

	if (peer == NULL || srq_ptr == NULL)
		return RPMA_E_INVAL;

	if (cfg == NULL)
		cfg = rpma_srq_cfg_default();

	struct ibv_srq *ibv_srq = NULL;
	struct rpma_cq *rcq = NULL;
	int ret = rpma_peer_create_srq(peer, cfg, &ibv_srq, &rcq);
	if (ret)
		return ret;

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

	if (srq_ptr == NULL)
		return RPMA_E_INVAL;

	struct rpma_srq *srq = *srq_ptr;
	int ret = 0;

	if (srq == NULL)
		return ret;

	ret = rpma_cq_delete(&srq->rcq);

	if (srq->ibv_srq) {
		errno = ibv_destroy_srq(srq->ibv_srq);
		if (!ret && errno) {
			RPMA_LOG_ERROR_WITH_ERRNO(errno, "ibv_destroy_srq()");
			ret = RPMA_E_PROVIDER;
		}
	}

	free(srq);
	*srq_ptr = NULL;

	RPMA_FAULT_INJECTION(RPMA_E_PROVIDER, {});
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

	if (srq == NULL || rcq_ptr == NULL)
		return RPMA_E_INVAL;

	*rcq_ptr = srq->rcq;

	RPMA_FAULT_INJECTION(RPMA_E_INVAL, {});
	return 0;
}
