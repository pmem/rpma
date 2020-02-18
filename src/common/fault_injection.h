// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2019-2020, Intel Corporation */

#ifndef CORE_FAULT_INJECTION
#define CORE_FAULT_INJECTION

#include <stdlib.h>

enum pmem_allocation_type { PMEM_MALLOC, PMEM_REALLOC };

#if FAULT_INJECTION
void core_inject_fault_at(enum pmem_allocation_type type,
	int nth, const char *at);

int core_fault_injection_enabled(void);

#else
static inline void
core_inject_fault_at(enum pmem_allocation_type type, int nth, const char *at)
{
	abort();
}

static inline int
core_fault_injection_enabled(void)
{
	return 0;
}
#endif

#endif
