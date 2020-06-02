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
	RPMA_INFO_PASSIVE, /* a passive side of the connection */
	RPMA_INFO_ACTIVE /* an active side of the connection */
};

struct rpma_info;

int rpma_info_new(const char *addr, const char *service,
		enum rpma_info_side side, struct rpma_info **info);

int rpma_info_delete(struct rpma_info **info);

int rpma_info_resolve_addr(struct rpma_info *info, struct rdma_cm_id *id);

int rdma_info_bind_addr(struct rpma_info *info);

#endif /* LIBRPMA_INFO_H */
