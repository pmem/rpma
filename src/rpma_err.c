// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2019-2022, Intel Corporation */

/*
 * rpma_err.c -- error-handling related librpma definitions
 */


#include "librpma.h"

/* public librpma API */

/*
 * rpma_err_2str -- return const string representation of an RPMA error
 */
const char *
rpma_err_2str(int ret)
{
	switch (ret) {
	case 0:
		return "Success";
	case RPMA_E_NOSUPP:
		return "Not supported";
	case RPMA_E_PROVIDER:
		return "Provider error occurred";
	case RPMA_E_NOMEM:
		return "Out of memory";
	case RPMA_E_INVAL:
		return "Invalid argument";
	case RPMA_E_NO_COMPLETION:
		return "No next completion available";
	case RPMA_E_NO_EVENT:
		return "No next event available";
	case RPMA_E_AGAIN:
		return "Temporary error, try again";
	case RPMA_E_SHARED_CHANNEL:
		return "Completion channel is shared";
	case RPMA_E_NOT_SHARED_CHNL:
		return "Completion channel is not shared";
	case RPMA_E_UNKNOWN:
	default:
		return "Unknown error";
	}
}
