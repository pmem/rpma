/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2019, Intel Corporation
 */

#include "valgrind_internal.h"

unsigned On_valgrind = 0;
unsigned On_pmemcheck = 0;
unsigned On_memcheck = 0;
unsigned On_helgrind = 0;
unsigned On_drd = 0;

void
set_valgrind_internals(void)
{
#if LIBRPMA_ANY_VG_TOOL_ENABLED
	On_valgrind = RUNNING_ON_VALGRIND;

	if (On_valgrind) {
		if (getenv("LIBRPMA_TRACER_PMEMCHECK"))
			On_pmemcheck = 1;
		else if (getenv("LIBRPMA_TRACER_MEMCHECK"))
			On_memcheck = 1;
		else if (getenv("LIBRPMA_TRACER_HELGRIND"))
			On_helgrind = 1;
		else if (getenv("LIBRPMA_TRACER_DRD"))
			On_drd = 1;
	}
#endif /* LIBRPMA_ANY_VG_TOOL_ENABLED */
}
