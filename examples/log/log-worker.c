// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020, Intel Corporation */

/*
 * log-worker.c -- an example how to use log
 */

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#include "../../src/common/log_internal.h"

void
log_worker_is_doing_something(void)
{
	rpma_log(RPMA_LOG_LEVEL_ERROR, __FILE__, __LINE__, __func__, \
			"Error message\n");
	rpma_log(RPMA_LOG_LEVEL_ERROR, NULL, __LINE__, __func__, "Error message\n");

	rpma_log(RPMA_LOG_LEVEL_WARNING, NULL, __LINE__, __func__, "Warning message\n");
	rpma_log(RPMA_LOG_LEVEL_DEBUG, __FILE__, __LINE__, __func__, \
			"Debug message\n");


	RPMA_LOG_NOTICE("Just a notice\n");
	RPMA_LOG_WARNING("Important warning about value: %d%c", 720401, '\n');
	RPMA_LOG_ERROR("Error due to order %x%c", 102, '\n');
	RPMA_PRINTF("I feel lost and do not know what to say, Master\n");

}
