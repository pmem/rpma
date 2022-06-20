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

#define RPMA_FAULT_INJECTION(ret_val, exit_code) \
	do { \
		int value = 0; \
		if (rpma_fault_injection(&value)) { \
			RPMA_LOG_ALWAYS("[#%i] [FAULT INJECTION: \"%s\"]", \
					value, rpma_err_2str(ret_val)); \
			{ exit_code; } \
			return ret_val; \
		} \
		if (value) \
			RPMA_LOG_ALWAYS("[#%i]", value); \
	} while (0)

#define RPMA_FAULT_INJECTION_GOTO(ret_val, goto_label) \
	RPMA_FAULT_INJECTION(ret_val, \
	{ \
		ret = ret_val; \
		goto goto_label; \
	});

int rpma_fault_injection(int *value);

#else

#define RPMA_FAULT_INJECTION(ret_val, exit_code) \
	do { } while (0)

#define RPMA_FAULT_INJECTION_GOTO(ret_val, goto_label) \
	do { } while (0)

#endif /* DEBUG_FAULT_INJECTION */

#endif /* LIBRPMA_DEBUG_H */
