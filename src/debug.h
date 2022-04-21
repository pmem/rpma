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
	RPMA_LOG_DEBUG("-")
#else
#define RPMA_DEBUG_TRACE
#endif /* DEBUG_LOG_TRACE */

#ifdef DEBUG_FAULT_INJECTION
#define RPMA_FAULT_INJECTION \
	do { \
		int ret = rpma_fault_injection(); \
		if (ret == RPMA_E_FAULT_INJECT) { \
			RPMA_LOG_DEBUG("[FAULT INJECTION]"); \
			return RPMA_E_FAULT_INJECT; \
		} \
		if (ret) \
			RPMA_LOG_DEBUG("[#%i]", ret); \
	} while (0);

int rpma_fault_injection(void);
#else
#define RPMA_FAULT_INJECTION
#endif /* DEBUG_FAULT_INJECTION */

#endif /* LIBRPMA_DEBUG_H */
