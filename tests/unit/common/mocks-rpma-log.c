/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2020, Intel Corporation
 */

/*
 * mocks-rpma-log.c -- librpma log.c module mocks
 */

#include "log_internal.h"

rpma_log_level Rpma_log_threshold[] = {
		RPMA_LOG_DISABLED, /* RPMA_LOG_THRESHOLD */
		RPMA_LOG_DISABLED /* RPMA_LOG_THRESHOLD_AUX */
};
