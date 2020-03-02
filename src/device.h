/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2020, Intel Corporation
 */

/*
 * device.h -- internal definitions for librpma device
 */
#ifndef RPMA_DEVICE_H
#define RPMA_DEVICE_H

#include <librpma.h>

struct rpma_device {
	struct ibv_context *verbs;
};

#endif /* RPMA_DEVICE_H */
