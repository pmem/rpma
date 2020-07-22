/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2020, Intel Corporation
 */

/*
 * rpma_err.h -- internal definitions for librpma error handling
 */

#ifndef LIBRPMA_ERR_H
#define LIBRPMA_ERR_H 1

extern int Rpma_provider_error;

#define RPMA_E_UNKNOWN_STR		"Unknown error"
#define RPMA_E_NOSUPP_STR		"Not supported"
#define RPMA_E_INVAL_STR		"Invalid argument"
#define RPMA_E_PROVIDER_STR		"Provider error occurred"
#define RPMA_E_NOMEM_STR		"Out of memory"
#define RPMA_E_NO_COMPLETION_STR	"No completion available"

#endif /* LIBRPMA_ERR_H */
