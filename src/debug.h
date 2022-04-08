/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright 2022, Intel Corporation */

/*
 * debug.h -- librpma debug internal definitions
 */

#ifndef LIBRPMA_DEBUG_H
#define LIBRPMA_DEBUG_H

#include "log_internal.h"

#ifdef DEBUG_LOG_TRACE
#define RPMA_LOG_TRACE \
	RPMA_LOG_DEBUG("-")
#else
#define RPMA_LOG_TRACE
#endif /* DEBUG_LOG_TRACE */

#ifdef DEBUG_FAULT_INJECTION
#define RPMA_FAULT_INJECTION \
	do { \
		if (rpma_fault_injection()) \
			return RPMA_E_FAULT_INJECT; \
	} while (0);

int rpma_fault_injection(void);
#else
#define RPMA_FAULT_INJECTION
#endif /* DEBUG_FAULT_INJECTION */

#endif /* LIBRPMA_DEBUG_H */
