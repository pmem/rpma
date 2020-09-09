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

#ifdef TEST_MOCK_ALLOC
#include "cmocka_alloc.h"
#endif

#define ON_OFF_TYPE2STR(var) ((var == RPMA_ON) ? "ON" : "OFF")

struct rpma_peer_cfg {
	enum rpma_on_off_type ddio;
	enum rpma_on_off_type auto_flush;
};

/* internal librpma API */

/*
 * rpma_peer_cfg_new -- create a new peer configuration object
 */
int
rpma_peer_cfg_new(struct rpma_peer_cfg **pcfg)
{
	struct rpma_peer_cfg *cfg = malloc(sizeof(struct rpma_peer_cfg));
	if (cfg == NULL)
		return RPMA_E_NOMEM;

	/* set default values */
	cfg->ddio = RPMA_ON;
	cfg->auto_flush = RPMA_OFF;
	*pcfg = cfg;
	return 0;
}

/*
 * rpma_peer_cfg_delete -- delete the peer configuration object
 */
int
rpma_peer_cfg_delete(struct rpma_peer_cfg **pcfg)
{
	free(*pcfg);
	*pcfg = NULL;
	return 0;
}

/*
 * rpma_peer_cfg_set_ddio -- declare the DDIO state
 */
int
rpma_peer_cfg_set_ddio(struct rpma_peer_cfg *pcfg, enum rpma_on_off_type state)
{
	if (state != RPMA_OFF && state != RPMA_ON)
		return RPMA_E_INVAL;

	pcfg->ddio = state;
	return 0;
}

/*
 * rpma_peer_cfg_set_auto_flush -- declare the auto flush state
 */
int
rpma_peer_cfg_set_auto_flush(struct rpma_peer_cfg *pcfg,
		enum rpma_on_off_type state)
{
	if (state != RPMA_OFF && state != RPMA_ON)
		return RPMA_E_INVAL;

	pcfg->auto_flush = state;
	return 0;
}

/*
 * rpma_peer_cfg_get_persistent_flush_supported -- check if persistent flush
 * is supported
 */
int
rpma_peer_cfg_get_persistent_flush_supported(struct rpma_peer_cfg *pcfg,
		enum rpma_on_off_type *state)
{
	if (pcfg->ddio == RPMA_OFF || pcfg->auto_flush == RPMA_ON)
		*state = RPMA_ON;
	else
		*state = RPMA_OFF;

	return 0;
}

/*
 * rpma_peer_cfg_get_descriptor -- get a descriptor of a peer configuration
 */
int
rpma_peer_cfg_get_descriptor(struct rpma_peer_cfg *pcfg,
		rpma_peer_cfg_descriptor *desc)
{
	if (pcfg == NULL || desc == NULL)
		return RPMA_E_INVAL;

	char *buff = (char *)desc;

	*((uint8_t *)buff) = (uint8_t)pcfg->ddio;
	buff += sizeof(uint8_t);
	*((uint8_t *)buff) = (uint8_t)pcfg->auto_flush;

	return 0;
}

/*
 * rpma_peer_cfg_from_descriptor -- create a peer configuration
 * from a descriptor
 */
int
rpma_peer_cfg_from_descriptor(rpma_peer_cfg_descriptor *desc,
		struct rpma_peer_cfg **pcfg)
{
	if (desc == NULL || pcfg == NULL)
		return RPMA_E_INVAL;

	char *buff = (char *)desc;

	uint8_t ddio = *(uint8_t *)buff;
	buff += sizeof(uint8_t);
	uint8_t auto_flush = *(uint8_t *)buff;

	if (ddio != RPMA_OFF && ddio != RPMA_ON) {
		RPMA_LOG_ERROR(
			"incorrect value of the DDIO state read from descriptor: %i",
			ddio);
		return RPMA_E_INVAL;
	}

	if (auto_flush != RPMA_OFF && auto_flush != RPMA_ON) {
		RPMA_LOG_ERROR(
			"incorrect value of the auto flush state read from descriptor: %i",
			auto_flush);
		return RPMA_E_INVAL;
	}

	struct rpma_peer_cfg *cfg = malloc(sizeof(struct rpma_peer_cfg));
	if (cfg == NULL)
		return RPMA_E_NOMEM;

	cfg->ddio = ddio;
	cfg->auto_flush = auto_flush;
	*pcfg = cfg;

	RPMA_LOG_INFO("new rpma_peer_cfg(ddio=%s, auto_flush=%s)",
			ON_OFF_TYPE2STR(ddio), ON_OFF_TYPE2STR(auto_flush));

	return 0;
}
