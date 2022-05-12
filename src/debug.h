/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright 2022, Intel Corporation */

/*
 * debug.h -- librpma debug internal definitions
 */

#ifndef LIBRPMA_DEBUG_H
#define LIBRPMA_DEBUG_H

#include "log_internal.h"

#ifdef DEBUG_LOG_TRACE
#define RPMA_DEBUG_TRACE \
	RPMA_LOG_ALWAYS("-")
#else
#define RPMA_DEBUG_TRACE \
	do { } while (0)
#endif /* DEBUG_LOG_TRACE */

#ifdef DEBUG_FAULT_INJECTION

#define RPMA_FAULT_INJECTION(exit_code) \
	do { \
		int value = 0; \
		if (rpma_fault_injection(&value)) { \
			RPMA_LOG_ALWAYS("[#%i] [FAULT INJECTION]", value); \
			int ret_fi = RPMA_E_FAULT_INJECT; \
			{ exit_code; } \
			return ret_fi; \
		} \
		if (value) \
			RPMA_LOG_ALWAYS("[#%i]", value); \
	} while (0)

int rpma_fault_injection(int *value);

#else

#define RPMA_FAULT_INJECTION(exit_code) \
	do { } while (0)

#endif /* DEBUG_FAULT_INJECTION */

#endif /* LIBRPMA_DEBUG_H */
