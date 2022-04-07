/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright 2022, Intel Corporation */

/*
 * comp_channel.h -- librpma completion channel related internal definitions
 */

#ifndef LIBRPMA_COMP_CHANNEL_H
#define LIBRPMA_COMP_CHANNEL_H

#include <infiniband/verbs.h>

#include "librpma.h"

/*
 * ERRORS
 * rpma_comp_channel_new() can fail with the following errors:
 *
 * - RPMA_E_PROVIDER - ibv_create_comp_channel(3) failed with a provider error
 */
int rpma_comp_channel_new(struct ibv_context *ibv_ctx,
		struct ibv_comp_channel **channel_ptr);

/*
 * ERRORS
 * rpma_comp_channel_destroy() can fail with the following errors:
 *
 * - RPMA_E_PROVIDER - ibv_destroy_comp_channel(3) failed with a provider error
 */
int rpma_comp_channel_destroy(struct ibv_comp_channel *channel);

#endif /* LIBRPMA_COMP_CHANNEL_H */
