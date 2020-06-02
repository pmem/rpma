/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2020, Intel Corporation
 */

/*
 * info.h -- librpma info-related internal definitions
 */

#ifndef LIBRPMA_INFO_H
#define LIBRPMA_INFO_H

#include <rdma/rdma_cma.h>

enum rpma_info_side {
	RPMA_INFO_PASSIVE,
	RPMA_INFO_ACTIVE
};

struct rpma_info;

/*
 * ERRORS
 * rpma_info_new() can fail with the following errors:
 *
 * - RPMA_E_INVAL - addr is NULL
 * - RPMA_E_INVAL - service is NULL
 * - RPMA_E_INVAL - **info_ptr is NULL
 * - RPMA_E_PROVIDER - address translation failed with error
 */
int rpma_info_new(const char *addr, const char *service,
		enum rpma_info_side side, struct rpma_info **info_ptr);

/*
 * ERRORS
 * rpma_info_delete() can fail with the following error:
 *
 * - RPMA_E_INVAL - info is NULL
 */
int rpma_info_delete(struct rpma_info **info);

/*
 * ERRORS
 * rpma_info_resolve_addr() can fail with the following error:
 *
 * - RPMA_E_PROVIDER - resolving the destination failed with error
 */
int rpma_info_resolve_addr(struct rdma_cm_id *id, struct rpma_info *info);

/*
 * ERRORS
 * rdma_info_bind_addr() can fail with the following error:
 *
 * - RPMA_E_PROVIDER - binding to address failed with error
 */
int rdma_info_bind_addr(struct rdma_cm_id *id, struct rpma_info *info);

#endif /* LIBRPMA_INFO_H */
