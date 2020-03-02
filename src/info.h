/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2020, Intel Corporation
 */

/*
 * info.h -- internal definitions for librpma info
 */
#ifndef RPMA_INFO_H
#define RPMA_INFO_H

#include <librpma.h>

#define RPMA_INFO_PASSIVE 1
#define RPMA_INFO_ACTIVE 0

struct rpma_info {
	const char *addr;
	const char *service;
	int passive;

	struct rdma_addrinfo *rai;
};

int info_resolve(struct rpma_info *info);
void info_free(struct rpma_info *info);
const char *info_dump(struct rpma_info *info);

#endif /* RPMA_INFO_H */
