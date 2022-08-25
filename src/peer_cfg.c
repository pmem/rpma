// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020-2022, Intel Corporation */

/*
 * peer_cfg.c -- librpma peer-configuration-related implementations
 */

#include <errno.h>
#include <inttypes.h>
#include <stdlib.h>
#include <stdbool.h>

#include "librpma.h"
#include "log_internal.h"
#include "debug.h"


#ifdef TEST_MOCK_ALLOC
#include "cmocka_alloc.h"
#endif

#ifdef ATOMIC_OPERATIONS_SUPPORTED
#include <stdatomic.h>
#endif /* ATOMIC_OPERATIONS_SUPPORTED */

#define SUPPORTED2STR(var) ((var) ? "supported" : "unsupported")

static bool RPMA_DEFAULT_DIRECT_WRITE_TO_PMEM = false;

struct rpma_peer_cfg {
#ifdef ATOMIC_OPERATIONS_SUPPORTED
	_Atomic
#endif /* ATOMIC_OPERATIONS_SUPPORTED */
	bool direct_write_to_pmem;
};

/* public librpma API */

/*
 * rpma_peer_cfg_new -- create a new peer configuration object
 */
int
rpma_peer_cfg_new(struct rpma_peer_cfg **pcfg_ptr)
{
	RPMA_DEBUG_TRACE;
	RPMA_FAULT_INJECTION(RPMA_E_NOMEM, {});

	if (pcfg_ptr == NULL)
		return RPMA_E_INVAL;

	struct rpma_peer_cfg *cfg = malloc(sizeof(struct rpma_peer_cfg));
	if (cfg == NULL)
		return RPMA_E_NOMEM;

	/* set default values */

#ifdef ATOMIC_OPERATIONS_SUPPORTED
	atomic_init(&cfg->direct_write_to_pmem, RPMA_DEFAULT_DIRECT_WRITE_TO_PMEM);
#else
	cfg->direct_write_to_pmem = RPMA_DEFAULT_DIRECT_WRITE_TO_PMEM;
#endif /* ATOMIC_OPERATIONS_SUPPORTED */
	*pcfg_ptr = cfg;
	return 0;
}

/*
 * rpma_peer_cfg_delete -- delete the peer configuration object
 */
int
rpma_peer_cfg_delete(struct rpma_peer_cfg **pcfg_ptr)
{
	RPMA_DEBUG_TRACE;

	if (pcfg_ptr == NULL)
		return RPMA_E_INVAL;

	free(*pcfg_ptr);
	*pcfg_ptr = NULL;

	RPMA_FAULT_INJECTION(RPMA_E_INVAL, {});
	return 0;
}

/*
 * rpma_peer_cfg_set_direct_write_to_pmem -- declare if direct write to PMEM is supported
 */
int
rpma_peer_cfg_set_direct_write_to_pmem(struct rpma_peer_cfg *pcfg, bool supported)
{
	RPMA_DEBUG_TRACE;
	RPMA_FAULT_INJECTION(RPMA_E_INVAL, {});

	if (pcfg == NULL)
		return RPMA_E_INVAL;

#ifdef ATOMIC_OPERATIONS_SUPPORTED
	atomic_store_explicit(&pcfg->direct_write_to_pmem, supported, __ATOMIC_SEQ_CST);
#else
	pcfg->direct_write_to_pmem = supported;
#endif /* ATOMIC_OPERATIONS_SUPPORTED */

	return 0;
}

/*
 * rpma_peer_cfg_get_direct_write_to_pmem -- check if direct write to PMEM is supported
 */
int
rpma_peer_cfg_get_direct_write_to_pmem(const struct rpma_peer_cfg *pcfg, bool *supported)
{
	RPMA_DEBUG_TRACE;
	RPMA_FAULT_INJECTION(RPMA_E_INVAL, {});

	if (pcfg == NULL || supported == NULL)
		return RPMA_E_INVAL;

#ifdef ATOMIC_OPERATIONS_SUPPORTED
	*supported = atomic_load_explicit((_Atomic bool *)&pcfg->direct_write_to_pmem,
			__ATOMIC_SEQ_CST);
#else
	*supported = pcfg->direct_write_to_pmem;
#endif /* ATOMIC_OPERATIONS_SUPPORTED */

	return 0;
}

/*
 * rpma_peer_cfg_get_descriptor -- get a descriptor of a peer configuration
 */
int
rpma_peer_cfg_get_descriptor(const struct rpma_peer_cfg *pcfg, void *desc)
{
	RPMA_DEBUG_TRACE;
	RPMA_FAULT_INJECTION(RPMA_E_INVAL, {});

	if (pcfg == NULL || desc == NULL)
		return RPMA_E_INVAL;

	bool direct_write_to_pmem;
	rpma_peer_cfg_get_direct_write_to_pmem(pcfg, &direct_write_to_pmem);
	*((uint8_t *)desc) = (uint8_t)direct_write_to_pmem;

	return 0;
}

/*
 * rpma_peer_cfg_get_descriptor_size -- get size of the peer configuration descriptor
 */
int
rpma_peer_cfg_get_descriptor_size(const struct rpma_peer_cfg *pcfg, size_t *desc_size)
{
	RPMA_DEBUG_TRACE;
	RPMA_FAULT_INJECTION(RPMA_E_INVAL, {});

	if (pcfg == NULL || desc_size == NULL)
		return RPMA_E_INVAL;

	*desc_size = sizeof(uint8_t);

	return 0;
}

/*
 * rpma_peer_cfg_from_descriptor -- create a peer configuration from a descriptor
 */
int
rpma_peer_cfg_from_descriptor(const void *desc, size_t desc_size, struct rpma_peer_cfg **pcfg_ptr)
{
	RPMA_DEBUG_TRACE;
	RPMA_FAULT_INJECTION(RPMA_E_INVAL, {});

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
