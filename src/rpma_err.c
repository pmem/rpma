/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2019-2020, Intel Corporation
 */

/*
 * rpma_err.c -- error-handling related librpma definitions
 */

#include "rpma_err.h"

#include "librpma.h"

int Rpma_provider_error = 0;

/* public librpma API */

/*
 * rpma_err_get_provider_error -- return the last provider error
 */
int
rpma_err_get_provider_error(void)
{
	return Rpma_provider_error;
}

/*
 * rpma_err_get_msg -- return the last error message
 */
const char *
rpma_err_get_msg(void)
{
	return "";
}

/*
 * rpma_e2str -- return const string representation of a RPMA error
 */
const char *
rpma_err_2str(int ret)
{
	switch (ret) {
	case RPMA_E_UNKNOWN:
		return "RPMA_E_UNKNOWN";
	case RPMA_E_NOSUPP:
		return "RPMA_E_NOSUPP";
	case RPMA_E_PROVIDER:
		return "RPMA_E_PROVIDER";
	case RPMA_E_NOMEM:
		return "RPMA_E_NOMEM";
	case RPMA_E_INVAL:
		return "RPMA_E_INVAL";
	default:
		return "unknown";
	}
}
