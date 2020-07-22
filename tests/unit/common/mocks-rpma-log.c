/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2020, Intel Corporation
 */

/*
 * mocks-rpma-log.c -- librpma log.c module mocks
 */

#include <librpma_log.h>

/*
 * rpma_log -- rpma_log() mock
 */
void
rpma_log(rpma_log_level level, const char *file_name, const int line_no,
	const char *function_name, const char *message_format, ...)
{
	;
}
