// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020, Intel Corporation */

/*
 * peer_cfg.c -- librpma peer-configuration-related implementations
 */

#include <errno.h>
#include <inttypes.h>
#include <stdlib.h>

#include "log_internal.h"
#include "peer_cfg.h"
#include "rpma_err.h"

#ifdef TEST_MOCK_ALLOC
#include "cmocka_alloc.h"
#endif

/* internal librpma API */

/*
 * rpma_peer_cfg_new -- create a new peer configuration object
 */
int
rpma_peer_cfg_new(struct rpma_peer_cfg **pcfg)
{
	return RPMA_E_NOSUPP;
}

/*
 * rpma_peer_cfg_delete -- delete the peer configuration object
 */
int
rpma_peer_cfg_delete(struct rpma_peer_cfg **pcfg)
{
	return RPMA_E_NOSUPP;
}

/*
 * rpma_peer_cfg_set_ddio -- declare the DDIO state
 */
int
rpma_peer_cfg_set_ddio(struct rpma_peer_cfg *pcfg, enum rpma_on_off_type state)
{
	return RPMA_E_NOSUPP;
}

/*
 * rpma_peer_cfg_set_auto_flush -- declare the auto flush state
 */
int
rpma_peer_cfg_set_auto_flush(struct rpma_peer_cfg *pcfg,
		enum rpma_on_off_type state)
{
	return RPMA_E_NOSUPP;
}

/*
 * rpma_peer_cfg_get_persistent_flush_supported -- XXX
 */
int
rpma_peer_cfg_get_persistent_flush_supported(struct rpma_peer_cfg *pcfg,
		enum rpma_on_off_type state)
{
	return RPMA_E_NOSUPP;
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
