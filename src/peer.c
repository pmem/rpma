// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020, Intel Corporation */

/*
 * peer.c -- librpma peer-related implementations
 */

#include <errno.h>
#include <inttypes.h>
#include <stdlib.h>

#include "conn_req.h"
#include "log_internal.h"
#include "peer.h"

#ifdef TEST_MOCK_ALLOC
#include "cmocka_alloc.h"
#endif

/* the maximum number of scatter/gather elements in any Work Request */
#define RPMA_MAX_SGE 1

/* the maximum message size (in bytes) that can be posted inline */
#define RPMA_MAX_INLINE_DATA 0

struct rpma_peer {
	struct ibv_pd *pd; /* a protection domain */

	int is_odp_supported; /* is On-Demand Paging supported */
};

/* internal librpma API */

/*
 * rpma_peer_create_qp -- allocate a QP associated with the CM ID
 *
 * ASSUMPTIONS
 * - cfg != NULL
 */
int
rpma_peer_create_qp(struct rpma_peer *peer, struct rdma_cm_id *id,
		struct ibv_cq *cq, const struct rpma_conn_cfg *cfg)
{
	if (peer == NULL || id == NULL || cq == NULL)
		return RPMA_E_INVAL;

	/* read SQ and RQ sizes from the configuration */
	uint32_t sq_size = 0;
	uint32_t rq_size = 0;
	(void) rpma_conn_cfg_get_sq_size(cfg, &sq_size);
	(void) rpma_conn_cfg_get_rq_size(cfg, &rq_size);

	struct ibv_qp_init_attr qp_init_attr;
	qp_init_attr.qp_context = NULL;
	qp_init_attr.send_cq = cq;
	qp_init_attr.recv_cq = cq;
	qp_init_attr.srq = NULL;
	qp_init_attr.cap.max_send_wr = sq_size;
	qp_init_attr.cap.max_recv_wr = rq_size;
	qp_init_attr.cap.max_send_sge = RPMA_MAX_SGE;
	qp_init_attr.cap.max_recv_sge = RPMA_MAX_SGE;
	qp_init_attr.cap.max_inline_data = RPMA_MAX_INLINE_DATA;
	/*
	 * Reliable Connection - since we are using e.g. IBV_WR_RDMA_READ.
	 * For details please see ibv_post_send(3).
	 */
	qp_init_attr.qp_type = IBV_QPT_RC;
	/*
	 * Every Work Request has to decide whether to generate CQ entry for its
	 * successful completion. Please see IBV_SEND_SIGNALED in
	 * ibv_post_send(3).
	 */
	qp_init_attr.sq_sig_all = 0;

	if (rdma_create_qp(id, peer->pd, &qp_init_attr)) {
		RPMA_LOG_ERROR_WITH_ERRNO(errno,
			"rdma_create_qp(max_send_wr=%" PRIu32
			", max_recv_wr=%" PRIu32
			", max_send/recv_sge=%i, max_inline_data=%i, qp_type=IBV_QPT_RC, sq_sig_all=0)",
			sq_size, rq_size, RPMA_MAX_SGE,
			RPMA_MAX_INLINE_DATA);
		return RPMA_E_PROVIDER;
	}

	/* XXX should we have to validate qp_init_attr output values? */

	return 0;
}

/*
 * Since rdma-core v27.0-105-g5a750676
 * ibv_reg_mr() has been defined as a macro
 * and its signature has been changed so that
 * the 'access' argument is of the 'unsigned int' type now:
 *
 * https://github.com/linux-rdma/rdma-core/commit/5a750676e8312715100900c6336bbc98577e082b
 */
#if defined(ibv_reg_mr)
#define RPMA_IBV_ACCESS(access) (unsigned)access
#else
#define RPMA_IBV_ACCESS(access) access
#endif

/*
 * rpma_peer_mr_reg -- register a memory region using ibv_reg_mr()
 */
int
rpma_peer_mr_reg(struct rpma_peer *peer, struct ibv_mr **ibv_mr_ptr,
		void *addr, size_t length, int access)
{
	*ibv_mr_ptr = ibv_reg_mr(peer->pd, addr, length,
					RPMA_IBV_ACCESS(access));
	if (*ibv_mr_ptr != NULL)
		return 0;

#ifdef ON_DEMAND_PAGING_SUPPORTED
	if (errno != EOPNOTSUPP) {
		RPMA_LOG_ERROR_WITH_ERRNO(errno, "ibv_reg_mr()");
		return RPMA_E_PROVIDER;
	}

	if (!peer->is_odp_supported) {
		RPMA_LOG_ERROR_WITH_ERRNO(errno,
			"Peer does not support On-Demand Paging: "
			"ibv_reg_mr(addr=%p, length=%zu, access=%i)",
			addr, length, access);
		return RPMA_E_PROVIDER;
	}

	/*
	 * If the registration failed with EOPNOTSUPP and On-Demand Paging is
	 * supported we can retry the memory registration with
	 * the IBV_ACCESS_ON_DEMAND flag.
	 */
	*ibv_mr_ptr = ibv_reg_mr(peer->pd, addr, length,
			RPMA_IBV_ACCESS(access | IBV_ACCESS_ON_DEMAND));
	if (*ibv_mr_ptr == NULL) {
		RPMA_LOG_ERROR_WITH_ERRNO(errno,
			"Memory registration with On-Demand Paging (maybe FSDAX?) support failed: "
			"ibv_reg_mr(addr=%p, length=%zu, acccess=%i|IBV_ACCESS_ON_DEMAND)",
			addr, length, access);
		return RPMA_E_PROVIDER;
	}

	return 0;
#else
	RPMA_LOG_ERROR_WITH_ERRNO(errno, "ibv_reg_mr()");
	return RPMA_E_PROVIDER;
#endif
}

/* public librpma API */

/*
 * rpma_peer_new -- create a new peer object encapsulating a newly allocated
 * verbs protection domain for provided ibv_context
 */
int
rpma_peer_new(struct ibv_context *ibv_ctx, struct rpma_peer **peer_ptr)
{
	int is_odp_supported = 0;
	int ret;

	if (ibv_ctx == NULL || peer_ptr == NULL)
		return RPMA_E_INVAL;

	ret = rpma_utils_ibv_context_is_odp_capable(ibv_ctx, &is_odp_supported);
	if (ret)
		return ret;

	/*
	 * The ibv_alloc_pd(3) manual page does not document that this function
	 * returns any error via errno but seemingly it is. For the usability
	 * sake, we try to deduce what really happened using the errno value.
	 * To make sure the errno value was set by the ibv_alloc_pd(3) function
	 * it is zeroed out before the function call.
	 */
	errno = 0;
	struct ibv_pd *pd = ibv_alloc_pd(ibv_ctx);
	if (pd == NULL) {
		if (errno == ENOMEM) {
			return RPMA_E_NOMEM;
		} else if (errno != 0) {
			RPMA_LOG_ERROR_WITH_ERRNO(errno, "ibv_alloc_pd()");
			return RPMA_E_PROVIDER;
		} else {
			return RPMA_E_UNKNOWN;
		}
	}

	struct rpma_peer *peer = malloc(sizeof(*peer));
	if (peer == NULL) {
		ret = RPMA_E_NOMEM;
		goto err_dealloc_pd;
	}

	peer->pd = pd;
	peer->is_odp_supported = is_odp_supported;
	*peer_ptr = peer;

	return 0;

err_dealloc_pd:
	ibv_dealloc_pd(pd);
	return ret;
}

/*
 * rpma_peer_delete -- attempt deallocating the peer-encapsulated verbs
 * protection domain; if succeeded delete the peer
 */
int
rpma_peer_delete(struct rpma_peer **peer_ptr)
{
	if (peer_ptr == NULL)
		return RPMA_E_INVAL;

	struct rpma_peer *peer = *peer_ptr;
	if (peer == NULL)
		return 0;

	int ret = ibv_dealloc_pd(peer->pd);
	if (ret) {
		RPMA_LOG_ERROR_WITH_ERRNO(errno, "ibv_dealloc_pd()");
		return RPMA_E_PROVIDER;
	}

	free(peer);
	*peer_ptr = NULL;

	return 0;
}
