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
	struct rpma_peer_cfg *cfg = malloc(sizeof(*cfg));
	if (cfg == NULL)
		return RPMA_E_NOMEM;

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
	pcfg->auto_flush = state;
	return 0;
}

/*
 * rpma_peer_cfg_get_persistent_flush_supported -- XXX
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
	return RPMA_E_NOSUPP;
}

/*
 * rpma_peer_cfg_from_descriptor -- create a peer configuration
 * from a descriptor
 */
int
rpma_peer_cfg_from_descriptor(rpma_peer_cfg_descriptor *desc,
		struct rpma_peer_cfg **pcfg)
{
	return RPMA_E_NOSUPP;
}
