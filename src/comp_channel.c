// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2022, Intel Corporation */

/*
 * comp_channel.c -- librpma completion channel related implementations
 */

#include <errno.h>
#include "comp_channel.h"
#include "log_internal.h"

/*
 * rpma_comp_channel_new -- create a completion channel
 *
 * ASSUMPTIONS
 * - ibv_ctx != NULL && channel_ptr != NULL
 */
int
rpma_comp_channel_new(struct ibv_context *ibv_ctx,
		struct ibv_comp_channel **channel_ptr)
{
	/* create a completion channel */
	*channel_ptr = ibv_create_comp_channel(ibv_ctx);
	if (*channel_ptr == NULL) {
		RPMA_LOG_ERROR_WITH_ERRNO(errno, "ibv_create_comp_channel()");
		return RPMA_E_PROVIDER;
	}

	return 0;
}

/*
 * rpma_comp_channel_destroy -- destroy a completion channel
 *
 * ASSUMPTIONS
 * - channel != NULL
 */
int
rpma_comp_channel_destroy(struct ibv_comp_channel *channel)
{
	errno = ibv_destroy_comp_channel(channel);
	if (errno) {
		RPMA_LOG_ERROR_WITH_ERRNO(errno, "ibv_destroy_comp_channel()");
		return RPMA_E_PROVIDER;
	}

	return 0;
}
