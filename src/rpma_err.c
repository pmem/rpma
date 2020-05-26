/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2019-2020, Intel Corporation
 */

/*
 * rpma_err.c -- error-handling related librpma definitions
 */

#include "librpma.h"

static int Rpmem_provider_errno = 0;

/*
 * rpma_err_get_provider_error -- return the last provider error
 */
int
rpma_err_get_provider_error(void)
{
	return Rpmem_provider_errno;
}

/*
 * rpma_err_get_msg -- return the last error message
 */
const char *
rpma_err_get_msg(void)
{
	return "";
}
