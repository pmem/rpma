// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020, Intel Corporation */

/*
 * peer_cfg.c -- librpma peer-configuration-related implementations
 */

#include <errno.h>
#include <inttypes.h>
#include <stdlib.h>

#include "librpma.h"
#include "log_internal.h"
#include "peer_cfg.h"
#include "rpma_err.h"
#include "stdbool.h"

#ifdef TEST_MOCK_ALLOC
#include "cmocka_alloc.h"
#endif

#define SUPPORTED2STR(var) ((var) ? "supported" : "unsupported")

struct rpma_peer_cfg {
	bool direct_write_to_pmem;
};

/* internal librpma API */

/*
 * rpma_peer_cfg_new -- create a new peer configuration object
 */
int
rpma_peer_cfg_new(struct rpma_peer_cfg **pcfg_ptr)
{
	if (pcfg_ptr == NULL)
		return RPMA_E_INVAL;

	struct rpma_peer_cfg *cfg = malloc(sizeof(struct rpma_peer_cfg));
	if (cfg == NULL)
		return RPMA_E_NOMEM;

	/* set default values */
	cfg->direct_write_to_pmem = false;
	*pcfg_ptr = cfg;
	return 0;
}

/*
 * rpma_peer_cfg_delete -- delete the peer configuration object
 */
int
rpma_peer_cfg_delete(struct rpma_peer_cfg **pcfg_ptr)
{
	if (pcfg_ptr == NULL)
		return RPMA_E_INVAL;

	free(*pcfg_ptr);
	*pcfg_ptr = NULL;
	return 0;
}

/*
 * rpma_peer_cfg_set_direct_write_to_pmem -- declare if direct write
 * to PMEM is supported
 */
int
rpma_peer_cfg_set_direct_write_to_pmem(struct rpma_peer_cfg *pcfg,
		bool supported)
{
	if (pcfg == NULL)
		return RPMA_E_INVAL;

	pcfg->direct_write_to_pmem = supported;
	return 0;
}

/*
 * rpma_peer_cfg_get_direct_write_to_pmem -- check if direct write
 * to PMEM is supported
 */
int
rpma_peer_cfg_get_direct_write_to_pmem(struct rpma_peer_cfg *pcfg,
		bool *supported)
{
	if (pcfg == NULL || supported == NULL)
		return RPMA_E_INVAL;

	*supported = pcfg->direct_write_to_pmem;
	return 0;
}

/*
 * rpma_peer_cfg_get_descriptor -- get a descriptor of a peer configuration
 */
int
rpma_peer_cfg_get_descriptor(struct rpma_peer_cfg *pcfg, void *desc)
{
	if (pcfg == NULL || desc == NULL)
		return RPMA_E_INVAL;

	*((uint8_t *)desc) = (uint8_t)pcfg->direct_write_to_pmem;

	return 0;
}

/*
 * rpma_peer_cfg_get_descriptor_size -- get size of the peer configuration
 * descriptor
 */
int
rpma_peer_cfg_get_descriptor_size(struct rpma_peer_cfg *pcfg, size_t *desc_size)
{
	if (pcfg == NULL || desc_size == NULL)
		return RPMA_E_INVAL;

	*desc_size = sizeof(uint8_t);

	return 0;
}

/*
 * rpma_peer_cfg_from_descriptor -- create a peer configuration
 * from a descriptor
 */
int
rpma_peer_cfg_from_descriptor(void *desc, size_t desc_size,
		struct rpma_peer_cfg **pcfg_ptr)
{
	if (desc == NULL || pcfg_ptr == NULL)
		return RPMA_E_INVAL;

	if (desc_size < sizeof(uint8_t)) {
		RPMA_LOG_ERROR(
			"incorrect size of the descriptor: %i bytes (should be at least %i bytes)",
			desc_size, sizeof(uint8_t));
		return RPMA_E_INVAL;
	}

	struct rpma_peer_cfg *cfg = malloc(sizeof(struct rpma_peer_cfg));
	if (cfg == NULL)
		return RPMA_E_NOMEM;

	cfg->direct_write_to_pmem = *(uint8_t *)desc;
	*pcfg_ptr = cfg;

	RPMA_LOG_INFO("new rpma_peer_cfg(direct_write_to_pmem=%s)",
			SUPPORTED2STR(cfg->direct_write_to_pmem));

	return 0;
}
