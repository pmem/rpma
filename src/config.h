/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2019-2020, Intel Corporation
 */

/*
 * config.h -- internal definitions for librpma config
 */
#ifndef RPMA_CONFIG_H
#define RPMA_CONFIG_H

#include <librpma.h>
#include <stdint.h>

struct rpma_config {
	char *addr;
	char *service;
	size_t msg_size;
	uint64_t send_queue_length;
	uint64_t recv_queue_length;
	rpma_malloc_func malloc;
	rpma_free_func free;
	unsigned flags;
};

#endif /* config.h */
