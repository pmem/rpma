/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright 2022, Intel Corporation */

/*
 * rpma.h -- librpma internal definitions
 */

#ifndef LIBRPMA_RPMA_H
#define LIBRPMA_RPMA_H

#include <infiniband/verbs.h>

#include "librpma.h"

/*
 * ERRORS
 * rpma_ibv_create_comp_channel() can fail with the following errors:
 *
 * - RPMA_E_PROVIDER - ibv_create_comp_channel(3) failed with a provider error
 */
int rpma_ibv_create_comp_channel(struct ibv_context *ibv_ctx,
		struct ibv_comp_channel **channel_ptr);

/*
 * ERRORS
 * rpma_ibv_destroy_comp_channel() can fail with the following errors:
 *
 * - RPMA_E_PROVIDER - ibv_destroy_comp_channel(3) failed with a provider error
 */
int rpma_ibv_destroy_comp_channel(struct ibv_comp_channel *channel);

#endif /* LIBRPMA_RPMA_H */
