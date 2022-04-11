// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2022, Intel Corporation */

/*
 * debug.c -- librpma debug implementations
 */

#include <stdlib.h>
#include "librpma.h"
#include "debug.h"

#ifdef DEBUG_FAULT_INJECTION
/*
 * rpma_fault_injection -- fail when counter reaches the value
 * defined by the RPMA_FAULT_INJECTION environment variable
 */
int
rpma_fault_injection(void)
{
	static int counter = 0;
	static int max_counter = 0;

	if (counter == 0 && getenv("RPMA_FAULT_INJECTION") != NULL) {
		max_counter = atoi(getenv("RPMA_FAULT_INJECTION"));
		if (max_counter <= 0) {
			max_counter = 0;
			counter = 1;
		}
	}

	if (max_counter) {
		if (counter > max_counter)
			return RPMA_E_FAULT_INJECT;

		return ++counter;
	}

	return 0;
}
#endif /* DEBUG_FAULT_INJECTION */
