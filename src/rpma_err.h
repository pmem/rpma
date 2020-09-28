/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright 2020, Intel Corporation */

/*
 * rpma_err.h -- internal definitions for librpma error handling
 */

#ifndef LIBRPMA_ERR_H
#define LIBRPMA_ERR_H 1

extern int Rpma_provider_error;

/*
 * ERRORS
 * rpma_err_get_provider_error() cannot fail.
 */
int rpma_err_get_provider_error(void);

#endif /* LIBRPMA_ERR_H */
