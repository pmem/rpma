// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020-2022, Intel Corporation */
/* Copyright 2021-2022, Fujitsu */

/*
 * conn_cfg.c -- librpma connection-configuration-related implementations
 */

#include <limits.h>
#include <stdlib.h>
#ifdef ATOMIC_OPERATIONS_SUPPORTED
#include <stdatomic.h>
#endif /* ATOMIC_OPERATIONS_SUPPORTED */
#include <rdma/rdma_cma.h>

#include "common.h"
#include "conn_cfg.h"
#include "conn_req.h"
#include "debug.h"
#include "librpma.h"
#include "log_internal.h"

#ifdef TEST_MOCK_ALLOC
#include "cmocka_alloc.h"
#endif

/*
 * For the simplicity sake, it is assumed all CQ/SQ/RQ default sizes are equal.
 */
#define RPMA_DEFAULT_Q_SIZE 10

/*
 * The default size of the receive CQ is 0, which means no receive CQ
 * is created for the connection.
 */
#define RPMA_DEFAULT_RCQ_SIZE	0

/*
 * By default the completion channel is NOT shared by CQ and RCQ.
 */
#define RPMA_DEFAULT_SHARED_COMPL_CHANNEL false

struct rpma_conn_cfg {
#ifdef ATOMIC_OPERATIONS_SUPPORTED
	_Atomic int timeout_ms;		/* connection establishment timeout */
	_Atomic uint32_t cq_size;	/* main CQ size */
	_Atomic uint32_t rcq_size;	/* receive CQ size */
	_Atomic uint32_t sq_size;	/* SQ size */
	_Atomic uint32_t rq_size;	/* RQ size */
	_Atomic bool shared_comp_channel; /* completion channel shared by CQ and RCQ */
	_Atomic uintptr_t srq;		/* shared RQ object of (struct rpma_srq *) type */
#else
	int timeout_ms;		/* connection establishment timeout */
	uint32_t cq_size;	/* main CQ size */
	uint32_t rcq_size;	/* receive CQ size */
	uint32_t sq_size;	/* SQ size */
	uint32_t rq_size;	/* RQ size */
	bool shared_comp_channel; /* completion channel shared by CQ and RCQ */
	uintptr_t srq;		/* shared RQ object of (struct rpma_srq *) type */
#endif /* ATOMIC_OPERATIONS_SUPPORTED */
};

static struct rpma_conn_cfg Conn_cfg_default  = {
	.timeout_ms = RPMA_DEFAULT_TIMEOUT_MS,
	.cq_size = RPMA_DEFAULT_Q_SIZE,
	.rcq_size = RPMA_DEFAULT_RCQ_SIZE,
	.sq_size = RPMA_DEFAULT_Q_SIZE,
	.rq_size = RPMA_DEFAULT_Q_SIZE,
	.shared_comp_channel = RPMA_DEFAULT_SHARED_COMPL_CHANNEL,
	.srq = 0
};

/* internal librpma API */

/*
 * rpma_conn_cfg_default -- return pointer to default connection configuration object
 */
struct rpma_conn_cfg *
rpma_conn_cfg_default()
{
	RPMA_DEBUG_TRACE;

	return &Conn_cfg_default;
}

/*
 * rpma_conn_cfg_get_cqe -- ibv_create_cq(..., int cqe, ...) compatible variant
 * of rpma_conn_cfg_get_cq_size(). Round down the cq_size when it is too big
 * for storing into an int type of value. Convert otherwise.
 */
void
rpma_conn_cfg_get_cqe(const struct rpma_conn_cfg *cfg, int *cqe)
{
	RPMA_DEBUG_TRACE;

	uint32_t cq_size = 0;
	(void) rpma_conn_cfg_get_cq_size(cfg, &cq_size);
	*cqe = CLIP_TO_INT(cq_size);
}

/*
 * rpma_conn_cfg_get_rcqe -- ibv_create_cq(..., int cqe, ...) compatible variant
 * of rpma_conn_cfg_get_rcq_size(). Round down the rcq_size when it is too big
 * for storing into an int type of value. Convert otherwise.
 */
void
rpma_conn_cfg_get_rcqe(const struct rpma_conn_cfg *cfg, int *rcqe)
{
	RPMA_DEBUG_TRACE;

	uint32_t rcq_size = 0;
	(void) rpma_conn_cfg_get_rcq_size(cfg, &rcq_size);
	*rcqe = CLIP_TO_INT(rcq_size);
}

/* public librpma API */

/*
 * rpma_conn_cfg_new -- create a new connection configuration object
 */
int
rpma_conn_cfg_new(struct rpma_conn_cfg **cfg_ptr)
{
	RPMA_DEBUG_TRACE;
	RPMA_FAULT_INJECTION(RPMA_E_NOMEM, {});

	if (cfg_ptr == NULL)
		return RPMA_E_INVAL;

	*cfg_ptr = malloc(sizeof(struct rpma_conn_cfg));
	if (*cfg_ptr == NULL)
		return RPMA_E_NOMEM;

#ifdef ATOMIC_OPERATIONS_SUPPORTED
	atomic_init(&(*cfg_ptr)->cq_size,
		atomic_load_explicit(&Conn_cfg_default.cq_size, __ATOMIC_SEQ_CST));
	atomic_init(&(*cfg_ptr)->rq_size,
		atomic_load_explicit(&Conn_cfg_default.rq_size, __ATOMIC_SEQ_CST));
	atomic_init(&(*cfg_ptr)->sq_size,
		atomic_load_explicit(&Conn_cfg_default.sq_size, __ATOMIC_SEQ_CST));
	atomic_init(&(*cfg_ptr)->timeout_ms,
		atomic_load_explicit(&Conn_cfg_default.timeout_ms, __ATOMIC_SEQ_CST));
	atomic_init(&(*cfg_ptr)->rcq_size,
		atomic_load_explicit(&Conn_cfg_default.rcq_size, __ATOMIC_SEQ_CST));
	atomic_init(&(*cfg_ptr)->shared_comp_channel,
		atomic_load_explicit(&Conn_cfg_default.shared_comp_channel, __ATOMIC_SEQ_CST));
	atomic_init(&(*cfg_ptr)->srq,
		atomic_load_explicit(&Conn_cfg_default.srq, __ATOMIC_SEQ_CST));
#else
	memcpy(*cfg_ptr, &Conn_cfg_default, sizeof(struct rpma_conn_cfg));
#endif /* ATOMIC_OPERATIONS_SUPPORTED */

	return 0;
}

/*
 * rpma_conn_cfg_delete -- delete the connection configuration object
 */
int
rpma_conn_cfg_delete(struct rpma_conn_cfg **cfg_ptr)
{
	RPMA_DEBUG_TRACE;

	if (cfg_ptr == NULL)
		return RPMA_E_INVAL;

	if (*cfg_ptr == NULL)
		return 0;

	free(*cfg_ptr);
	*cfg_ptr = NULL;

	RPMA_FAULT_INJECTION(RPMA_E_INVAL, {});
	return 0;
}

/*
 * rpma_conn_cfg_set_timeout -- set connection establishment timeout
 */
int
rpma_conn_cfg_set_timeout(struct rpma_conn_cfg *cfg, int timeout_ms)
{
	RPMA_DEBUG_TRACE;
	RPMA_FAULT_INJECTION(RPMA_E_INVAL, {});

	if (cfg == NULL || timeout_ms < 0)
		return RPMA_E_INVAL;

#ifdef ATOMIC_OPERATIONS_SUPPORTED
	atomic_store_explicit(&cfg->timeout_ms, timeout_ms, __ATOMIC_SEQ_CST);
#else
	cfg->timeout_ms = timeout_ms;
#endif /* ATOMIC_OPERATIONS_SUPPORTED */

	return 0;
}

/*
 * rpma_conn_cfg_get_timeout -- get connection establishment timeout
 */
int
rpma_conn_cfg_get_timeout(const struct rpma_conn_cfg *cfg, int *timeout_ms)
{
	RPMA_DEBUG_TRACE;

	if (cfg == NULL || timeout_ms == NULL)
		return RPMA_E_INVAL;

#ifdef ATOMIC_OPERATIONS_SUPPORTED
	*timeout_ms = atomic_load_explicit((_Atomic int *)&cfg->timeout_ms, __ATOMIC_SEQ_CST);
#else
	*timeout_ms = cfg->timeout_ms;
#endif /* ATOMIC_OPERATIONS_SUPPORTED */

	RPMA_FAULT_INJECTION(RPMA_E_INVAL, {});
	return 0;
}

/*
 * rpma_conn_cfg_set_cq_size -- set CQ size for the connection
 */
int
rpma_conn_cfg_set_cq_size(struct rpma_conn_cfg *cfg, uint32_t cq_size)
{
	RPMA_DEBUG_TRACE;
	RPMA_FAULT_INJECTION(RPMA_E_INVAL, {});

	if (cfg == NULL)
		return RPMA_E_INVAL;

#ifdef ATOMIC_OPERATIONS_SUPPORTED
	atomic_store_explicit(&cfg->cq_size, cq_size, __ATOMIC_SEQ_CST);
#else
	cfg->cq_size = cq_size;
#endif /* ATOMIC_OPERATIONS_SUPPORTED */

	return 0;
}

/*
 * rpma_conn_cfg_get_cq_size -- get CQ size for the connection
 */
int
rpma_conn_cfg_get_cq_size(const struct rpma_conn_cfg *cfg, uint32_t *cq_size)
{
	RPMA_DEBUG_TRACE;
	/* fault injection is located at the end of this function - see the comment */

	if (cfg == NULL || cq_size == NULL)
		return RPMA_E_INVAL;

#ifdef ATOMIC_OPERATIONS_SUPPORTED
	*cq_size = atomic_load_explicit((_Atomic uint32_t *)&cfg->cq_size, __ATOMIC_SEQ_CST);
#else
	*cq_size = cfg->cq_size;
#endif /* ATOMIC_OPERATIONS_SUPPORTED */

	/*
	 * This function is used as void in rpma_conn_cfg_get_cqe() and therefore it has to return
	 * the correct value of size of CQ, if it fails because of fault injection.
	 */
	RPMA_FAULT_INJECTION(RPMA_E_INVAL, {});
	return 0;
}

/*
 * rpma_conn_cfg_set_rcq_size -- set receive CQ size for the connection
 */
int
rpma_conn_cfg_set_rcq_size(struct rpma_conn_cfg *cfg, uint32_t rcq_size)
{
	RPMA_DEBUG_TRACE;
	RPMA_FAULT_INJECTION(RPMA_E_INVAL, {});

	if (cfg == NULL)
		return RPMA_E_INVAL;

#ifdef ATOMIC_OPERATIONS_SUPPORTED
	atomic_store_explicit(&cfg->rcq_size, rcq_size, __ATOMIC_SEQ_CST);
#else
	cfg->rcq_size = rcq_size;
#endif /* ATOMIC_OPERATIONS_SUPPORTED */

	return 0;
}

/*
 * rpma_conn_cfg_get_rcq_size -- get receive CQ size for the connection
 */
int
rpma_conn_cfg_get_rcq_size(const struct rpma_conn_cfg *cfg, uint32_t *rcq_size)
{
	RPMA_DEBUG_TRACE;
	/* fault injection is located at the end of this function - see the comment */

	if (cfg == NULL || rcq_size == NULL)
		return RPMA_E_INVAL;

#ifdef ATOMIC_OPERATIONS_SUPPORTED
	*rcq_size = atomic_load_explicit((_Atomic uint32_t *)&cfg->rcq_size, __ATOMIC_SEQ_CST);
#else
	*rcq_size = cfg->rcq_size;
#endif /* ATOMIC_OPERATIONS_SUPPORTED */

	/*
	 * This function is used as void in rpma_conn_cfg_get_rcqe() and therefore it has to return
	 * the correct value of size of RCQ, if it fails because of fault injection.
	 */
	RPMA_FAULT_INJECTION(RPMA_E_INVAL, {});
	return 0;
}

/*
 * rpma_conn_cfg_set_sq_size -- set SQ size for the connection
 */
int
rpma_conn_cfg_set_sq_size(struct rpma_conn_cfg *cfg, uint32_t sq_size)
{
	RPMA_DEBUG_TRACE;
	RPMA_FAULT_INJECTION(RPMA_E_INVAL, {});

	if (cfg == NULL)
		return RPMA_E_INVAL;

#ifdef ATOMIC_OPERATIONS_SUPPORTED
	atomic_store_explicit(&cfg->sq_size, sq_size, __ATOMIC_SEQ_CST);
#else
	cfg->sq_size = sq_size;
#endif /* ATOMIC_OPERATIONS_SUPPORTED */

	return 0;
}

/*
 * rpma_conn_cfg_get_sq_size -- get SQ size for the connection
 */
int
rpma_conn_cfg_get_sq_size(const struct rpma_conn_cfg *cfg, uint32_t *sq_size)
{
	RPMA_DEBUG_TRACE;
	/* fault injection is located at the end of this function - see the comment */

	if (cfg == NULL || sq_size == NULL)
		return RPMA_E_INVAL;

#ifdef ATOMIC_OPERATIONS_SUPPORTED
	*sq_size = atomic_load_explicit((_Atomic uint32_t *)&cfg->sq_size, __ATOMIC_SEQ_CST);
#else
	*sq_size = cfg->sq_size;
#endif /* ATOMIC_OPERATIONS_SUPPORTED */

	/*
	 * This function is used as void in rpma_peer_setup_qp() and therefore it has to return
	 * the correct value of size of SQ, if it fails because of fault injection.
	 */
	RPMA_FAULT_INJECTION(RPMA_E_INVAL, {});
	return 0;
}

/*
 * rpma_conn_cfg_set_rq_size -- set RQ size for the connection
 */
int
rpma_conn_cfg_set_rq_size(struct rpma_conn_cfg *cfg, uint32_t rq_size)
{
	RPMA_DEBUG_TRACE;
	RPMA_FAULT_INJECTION(RPMA_E_INVAL, {});

	if (cfg == NULL)
		return RPMA_E_INVAL;

#ifdef ATOMIC_OPERATIONS_SUPPORTED
	atomic_store_explicit(&cfg->rq_size, rq_size, __ATOMIC_SEQ_CST);
#else
	cfg->rq_size = rq_size;
#endif /* ATOMIC_OPERATIONS_SUPPORTED */

	return 0;
}

/*
 * rpma_conn_cfg_get_rq_size -- get RQ size for the connection
 */
int
rpma_conn_cfg_get_rq_size(const struct rpma_conn_cfg *cfg, uint32_t *rq_size)
{
	RPMA_DEBUG_TRACE;
	/* fault injection is located at the end of this function - see the comment */

	if (cfg == NULL || rq_size == NULL)
		return RPMA_E_INVAL;

#ifdef ATOMIC_OPERATIONS_SUPPORTED
	*rq_size = atomic_load_explicit((_Atomic uint32_t *)&cfg->rq_size, __ATOMIC_SEQ_CST);
#else
	*rq_size = cfg->rq_size;
#endif /* ATOMIC_OPERATIONS_SUPPORTED */

	/*
	 * This function is used as void in rpma_peer_setup_qp() and therefore it has to return
	 * the correct value of size of RQ, if it fails because of fault injection.
	 */
	RPMA_FAULT_INJECTION(RPMA_E_INVAL, {});
	return 0;
}

/*
 * rpma_conn_cfg_set_compl_channel -- set if the completion channel is shared by CQ and RCQ
 */
int
rpma_conn_cfg_set_compl_channel(struct rpma_conn_cfg *cfg, bool shared)
{
	RPMA_DEBUG_TRACE;
	RPMA_FAULT_INJECTION(RPMA_E_INVAL, {});

	if (cfg == NULL)
		return RPMA_E_INVAL;

#ifdef ATOMIC_OPERATIONS_SUPPORTED
	atomic_store_explicit(&cfg->shared_comp_channel, shared, __ATOMIC_SEQ_CST);
#else
	cfg->shared_comp_channel = shared;
#endif /* ATOMIC_OPERATIONS_SUPPORTED */

	return 0;
}

/*
 * rpma_conn_cfg_get_compl_channel -- get if the completion channel is shared by CQ and RCQ
 */
int
rpma_conn_cfg_get_compl_channel(const struct rpma_conn_cfg *cfg, bool *shared)
{
	RPMA_DEBUG_TRACE;

	if (cfg == NULL || shared == NULL)
		return RPMA_E_INVAL;

#ifdef ATOMIC_OPERATIONS_SUPPORTED
	*shared = atomic_load_explicit((_Atomic bool *)&cfg->shared_comp_channel, __ATOMIC_SEQ_CST);
#else
	*shared = cfg->shared_comp_channel;
#endif /* ATOMIC_OPERATIONS_SUPPORTED */

	RPMA_FAULT_INJECTION(RPMA_E_INVAL, {});
	return 0;
}

/*
 * rpma_conn_cfg_set_srq -- set a shared RQ object for the connection
 */
int
rpma_conn_cfg_set_srq(struct rpma_conn_cfg *cfg, struct rpma_srq *srq)
{
	RPMA_DEBUG_TRACE;
	RPMA_FAULT_INJECTION(RPMA_E_INVAL, {});

	if (cfg == NULL || srq == NULL)
		return RPMA_E_INVAL;

#ifdef ATOMIC_OPERATIONS_SUPPORTED
	atomic_store_explicit(&cfg->srq, (uintptr_t)srq, __ATOMIC_SEQ_CST);
#else
	cfg->srq = (uintptr_t)srq;
#endif /* ATOMIC_OPERATIONS_SUPPORTED */

	return 0;
}

/*
 * rpma_conn_cfg_get_srq -- get the shared RQ object from the connection
 */
int
rpma_conn_cfg_get_srq(const struct rpma_conn_cfg *cfg, struct rpma_srq **srq_ptr)
{
	RPMA_DEBUG_TRACE;

	if (cfg == NULL || srq_ptr == NULL)
		return RPMA_E_INVAL;

#ifdef ATOMIC_OPERATIONS_SUPPORTED
	*srq_ptr = (struct rpma_srq *)atomic_load_explicit((_Atomic uintptr_t *)&cfg->srq,
			__ATOMIC_SEQ_CST);
#else
	*srq_ptr = (struct rpma_srq *)cfg->srq;
#endif

	RPMA_FAULT_INJECTION(RPMA_E_INVAL, {});
	return 0;
}
