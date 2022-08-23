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
 * rpma_fault_injection -- fail when counter reaches the value defined by the RPMA_FAULT_INJECTION
 * environment variable
 */
int
rpma_fault_injection(int *value)
{
	static int counter = 0;
	static int fault_injection = 0;

	if (counter == 0 && getenv("RPMA_FAULT_INJECTION") != NULL) {
		fault_injection = atoi(getenv("RPMA_FAULT_INJECTION"));
		if (fault_injection <= 0) {
			fault_injection = 0;
			counter = 1;
		}
	}

	if (fault_injection) {
		*value = ++counter;
		if (counter == fault_injection)
			return -1;
	}

	return 0;
}
#endif /* DEBUG_FAULT_INJECTION */
