// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2019-2020, Intel Corporation */

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
 * rpma_e2str -- return const string representation of an RPMA error
 */
const char *
rpma_err_2str(int ret)
{
	switch (ret) {
	case RPMA_E_NOSUPP:
		return "Not supported";
	case RPMA_E_PROVIDER:
		return "Provider error occurred";
	case RPMA_E_NOMEM:
		return "Out of memory";
	case RPMA_E_INVAL:
		return "Invalid argument";
	case RPMA_E_UNKNOWN:
	default:
		return "Unknown error";
	}
}
