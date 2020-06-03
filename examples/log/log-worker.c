// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020, Intel Corporation */

/*
 * log-worker.c -- an example of how the internal API of the log module is used
 * by the RPMA library
 */

#include <stdio.h>

#include "log_internal.h"

void
log_worker_is_doing_something(void)
{
	RPMA_LOG_NOTICE("Just a notice");
	RPMA_LOG_WARNING("Important warning about value: %d", 720401);
	RPMA_LOG_ERROR("Error due to order %x", 102);
	RPMA_PRINTF("PRINTF message requires explicit '\\n' at the end\n");
}
