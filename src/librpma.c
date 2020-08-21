// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2019-2020, Intel Corporation */

/*
 * librpma.c -- entry points for librpma
 */

#include "librpma.h"
#include "log_internal.h"

/*
 * librpma_init -- load-time initialization for librpma
 *
 * Called automatically by the run-time loader.
 */
#ifdef MOCK_CONSTRUCTOR
void
#else
__attribute__((constructor)) static void
#endif
librpma_init(void)
{
	rpma_log_init();
}

/*
 * librpma_fini -- librpma cleanup routine
 *
 * Called automatically when the process terminates.
 */
#ifdef MOCK_CONSTRUCTOR
void
#else
__attribute__((destructor)) static void
#endif
librpma_fini(void)
{
	rpma_log_fini();
}
