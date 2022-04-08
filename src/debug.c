// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2022, Intel Corporation */

/*
 * debug.c -- librpma debug implementations
 */

#include <stdlib.h>

#include "librpma.h"
#include "debug.h"

#define DEBUG_FAULT_INJECTION 1

#ifdef DEBUG_FAULT_INJECTION

/*
 * rpma_fault_injection -- fail when counter reaches defined value
 */
int
rpma_fault_injection(void)
{
	static int global_counter = 0;
	static int max_global_counter = 0;

	if (global_counter == 0 && getenv("RPMA_FAULT_INJECTION") != NULL) {
		max_global_counter = atoi(getenv("RPMA_FAULT_INJECTION"));
		if (max_global_counter <= 0) {
			max_global_counter = 0;
			global_counter = 1;
		}
	}

	if (max_global_counter) {
		if (global_counter > max_global_counter)
			return RPMA_E_FAULT_INJECT;

		global_counter++;
	}

	return 0;
}

#endif /* DEBUG_FAULT_INJECTION */
