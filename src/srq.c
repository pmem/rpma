// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2022, Fujitsu */

/*
 * srq.c -- librpma shared-RQ-related implementations
 */

#include <stdlib.h>
#include <string.h>

#include "cq.h"
#include "librpma.h"
#include "log_internal.h"
#include "peer.h"
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
	if (peer == NULL || srq_ptr == NULL)
		return RPMA_E_INVAL;

	if (cfg == NULL)
		cfg = rpma_srq_cfg_default();

	/* read shared RQ size from the configuration */
	uint32_t rq_size;
	(void) rpma_srq_cfg_get_rq_size(cfg, &rq_size);

	struct ibv_srq *ibv_srq = NULL;
	struct ibv_srq_init_attr srq_init_attr;
	srq_init_attr.srq_context = NULL;
	srq_init_attr.attr.max_wr = rq_size;
	srq_init_attr.attr.max_sge = 1;
	srq_init_attr.attr.srq_limit = 0;

	ibv_srq = ibv_create_srq(peer->pd, &srq_init_attr);
	if (ibv_srq == NULL) {
		RPMA_LOG_ERROR_WITH_ERRNO(errno, "ibv_create_srq()");
		return RPMA_E_PROVIDER;
	}

	/* read shared receive CQ size from the configuration */
	int rcqe;
	(void) rpma_srq_cfg_get_rcqe(cfg, &rcqe);

	struct rpma_cq *rcq = NULL;
	int ret = rpma_cq_new(peer->pd->context, rcqe, NULL, &rcq);
	if (ret)
		goto err_srq_delete;

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
	if (srq_ptr == NULL)
		return RPMA_E_INVAL;

	struct rpma_srq *srq = *srq_ptr;
	int ret = 0;

	if (srq == NULL)
		return ret;

	if (srq->rcq)
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

	return ret;
}
