// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2022, Fujitsu */
/* Copyright 2022, Intel Corporation */

/*
 * srq_cfg.c -- librpma shared-RQ-configuration-related implementations
 */

#include <limits.h>
#include <stdlib.h>
#ifdef ATOMIC_OPERATIONS_SUPPORTED
#include <stdatomic.h>
#endif /* ATOMIC_OPERATIONS_SUPPORTED */

#include "common.h"
#include "debug.h"
#include "librpma.h"

#ifdef TEST_MOCK_ALLOC
#include "cmocka_alloc.h"
#endif

#define RPMA_DEFAULT_SRQ_SIZE	20

struct rpma_srq_cfg {
#ifdef ATOMIC_OPERATIONS_SUPPORTED
	_Atomic uint32_t rq_size;
	_Atomic uint32_t rcq_size;
#else
	uint32_t rq_size;
	uint32_t rcq_size;
#endif /* ATOMIC_OPERATIONS_SUPPORTED */
};

static struct rpma_srq_cfg Srq_cfg_default  = {
	.rq_size = RPMA_DEFAULT_SRQ_SIZE,
	.rcq_size = RPMA_DEFAULT_SRQ_SIZE
};

/* internal librpma API */

/*
 * rpma_srq_cfg_default -- return pointer to default share RQ configuration
 * object
 */
struct rpma_srq_cfg *
rpma_srq_cfg_default()
{
	RPMA_DEBUG_TRACE;

	return &Srq_cfg_default;
}

/*
 * rpma_srq_cfg_get_rcqe -- ibv_create_cq(..., int cqe, ...) compatible variant
 * of rpma_srq_cfg_get_rcq_size(). Round down the rcq_size when it is too big
 * for storing into an int type of value. Convert otherwise.
 */
void
rpma_srq_cfg_get_rcqe(const struct rpma_srq_cfg *cfg, int *rcqe)
{
	RPMA_DEBUG_TRACE;

	uint32_t rcq_size = 0;
	(void) rpma_srq_cfg_get_rcq_size(cfg, &rcq_size);
	*rcqe = CLIP_TO_INT(rcq_size);
}

/* public librpma API */

/*
 * rpma_srq_cfg_new -- create a new shared RQ configuration
 */
int
rpma_srq_cfg_new(struct rpma_srq_cfg **cfg_ptr)
{
	RPMA_DEBUG_TRACE;
	RPMA_FAULT_INJECTION(RPMA_E_INVAL, {});

	if (cfg_ptr == NULL)
		return RPMA_E_INVAL;

	RPMA_FAULT_INJECTION(RPMA_E_NOMEM, {});
	*cfg_ptr = malloc(sizeof(struct rpma_srq_cfg));
	if (*cfg_ptr == NULL)
		return RPMA_E_NOMEM;

#ifdef ATOMIC_OPERATIONS_SUPPORTED
	atomic_init(&(*cfg_ptr)->rq_size,
		atomic_load_explicit(&Srq_cfg_default.rq_size, __ATOMIC_SEQ_CST));
	atomic_init(&(*cfg_ptr)->rcq_size,
		atomic_load_explicit(&Srq_cfg_default.rcq_size, __ATOMIC_SEQ_CST));
#else
	memcpy(*cfg_ptr, &Srq_cfg_default, sizeof(struct rpma_srq_cfg));
#endif /* ATOMIC_OPERATIONS_SUPPORTED */

	return 0;
}

/*
 * rpma_srq_cfg_delete -- delete the shared RQ configuration
 */
int
rpma_srq_cfg_delete(struct rpma_srq_cfg **cfg_ptr)
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
 * rpma_srq_cfg_set_rq_size -- set shared RQ size
 */
int
rpma_srq_cfg_set_rq_size(struct rpma_srq_cfg *cfg, uint32_t rq_size)
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
 * rpma_srq_cfg_get_rq_size -- get shared RQ size
 */
int
rpma_srq_cfg_get_rq_size(const struct rpma_srq_cfg *cfg, uint32_t *rq_size)
{
	RPMA_DEBUG_TRACE;

	if (cfg == NULL || rq_size == NULL)
		return RPMA_E_INVAL;

#ifdef ATOMIC_OPERATIONS_SUPPORTED
	*rq_size = atomic_load_explicit((_Atomic uint32_t *)&cfg->rq_size, __ATOMIC_SEQ_CST);
#else
	*rq_size = cfg->rq_size;
#endif /* ATOMIC_OPERATIONS_SUPPORTED */

	RPMA_FAULT_INJECTION(RPMA_E_INVAL, {});
	return 0;
}

/*
 * rpma_srq_cfg_set_rcq_size -- set shared receive CQ size
 */
int
rpma_srq_cfg_set_rcq_size(struct rpma_srq_cfg *cfg, uint32_t rcq_size)
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
 * rpma_srq_cfg_get_rcq_size -- get shared receive CQ size
 */
int
rpma_srq_cfg_get_rcq_size(const struct rpma_srq_cfg *cfg, uint32_t *rcq_size)
{
	RPMA_DEBUG_TRACE;

	if (cfg == NULL || rcq_size == NULL)
		return RPMA_E_INVAL;

#ifdef ATOMIC_OPERATIONS_SUPPORTED
	*rcq_size = atomic_load_explicit((_Atomic uint32_t *)&cfg->rcq_size, __ATOMIC_SEQ_CST);
#else
	*rcq_size = cfg->rcq_size;
#endif /* ATOMIC_OPERATIONS_SUPPORTED */

	RPMA_FAULT_INJECTION(RPMA_E_INVAL, {});
	return 0;
}
