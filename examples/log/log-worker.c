// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020, Intel Corporation */

/*
 * log-worker.c -- an example how to use log internal API
 */

#include <stdio.h>

#include "log_internal.h"

void
log_worker_is_doing_something(void)
{
	RPMA_LOG_WARNING("Important warning about value: %d%c", 720401, '\n');
	RPMA_LOG_ERROR("Error due to order %x%c", 102, '\n');
	RPMA_PRINTF("I feel lost and do not know what to say, Master\n");
	RPMA_LOG_NOTICE("Just a notice");
}
