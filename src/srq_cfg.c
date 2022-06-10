// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2022, Fujitsu */

/*
 * srq_cfg.c -- librpma shared-RQ-configuration-related implementations
 */

#include <limits.h>
#include <stdlib.h>

#include "common.h"
#include "librpma.h"

#ifdef TEST_MOCK_ALLOC
#include "cmocka_alloc.h"
#endif

#define RPMA_DEFAULT_SRQ_SIZE	100

struct rpma_srq_cfg {
	uint32_t rq_size;
	uint32_t rcq_size;
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
	return &Srq_cfg_default;
}

/*
 * rpma_srq_cfg_get_rcqe -- ibv_create_cq(..., int cqe, ...) compatible variant
 * of rpma_srq_cfg_get_rcq_size(). Round down the rcq_size when it is too big
 * for storing into an int type of value. Convert otherwise.
 */
int
rpma_srq_cfg_get_rcqe(const struct rpma_srq_cfg *cfg, int *rcqe)
{
	if (rcqe == NULL)
		return RPMA_E_INVAL;

	uint32_t rcq_size;
	int ret = rpma_srq_cfg_get_rcq_size(cfg, &rcq_size);
	if (ret)
		return ret;

	*rcqe = CLIP_TO_INT(rcq_size);

	return 0;
}

/* public librpma API */

/*
 * rpma_srq_cfg_new -- create a new shared RQ configuration
 */
int
rpma_srq_cfg_new(struct rpma_srq_cfg **cfg_ptr)
{
	if (cfg_ptr == NULL)
		return RPMA_E_INVAL;

	*cfg_ptr = malloc(sizeof(struct rpma_srq_cfg));
	if (*cfg_ptr == NULL)
		return RPMA_E_NOMEM;

	memcpy(*cfg_ptr, &Srq_cfg_default, sizeof(struct rpma_srq_cfg));

	return 0;
}

/*
 * rpma_srq_cfg_delete -- delete the shared RQ configuration
 */
int
rpma_srq_cfg_delete(struct rpma_srq_cfg **cfg_ptr)
{
	if (cfg_ptr == NULL)
		return RPMA_E_INVAL;

	if (*cfg_ptr == NULL)
		return 0;

	free(*cfg_ptr);
	*cfg_ptr = NULL;

	return 0;
}

/*
 * rpma_srq_cfg_set_rq_size -- set shared RQ size
 */
int
rpma_srq_cfg_set_rq_size(struct rpma_srq_cfg *cfg, uint32_t rq_size)
{
	if (cfg == NULL)
		return RPMA_E_INVAL;

	cfg->rq_size = rq_size;

	return 0;
}

/*
 * rpma_srq_cfg_get_rq_size -- get shared RQ size
 */
int
rpma_srq_cfg_get_rq_size(const struct rpma_srq_cfg *cfg, uint32_t *rq_size)
{
	if (cfg == NULL || rq_size == NULL)
		return RPMA_E_INVAL;

	*rq_size = cfg->rq_size;

	return 0;
}

/*
 * rpma_srq_cfg_set_rcq_size -- set shared receive CQ size
 */
int
rpma_srq_cfg_set_rcq_size(struct rpma_srq_cfg *cfg, uint32_t rcq_size)
{
	if (cfg == NULL)
		return RPMA_E_INVAL;

	cfg->rcq_size = rcq_size;

	return 0;
}

/*
 * rpma_srq_cfg_get_rcq_size -- get shared receive CQ size
 */
int
rpma_srq_cfg_get_rcq_size(const struct rpma_srq_cfg *cfg, uint32_t *rcq_size)
{
	if (cfg == NULL || rcq_size == NULL)
		return RPMA_E_INVAL;

	*rcq_size = cfg->rcq_size;

	return 0;
}
