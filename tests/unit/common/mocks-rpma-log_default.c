// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020, Intel Corporation */

/*
 * mocks-rpma-log_default.c -- librpma log_default.c module mocks
 */

#include "cmocka_headers.h"
#include "log_default.h"

/*
 * rpma_log_default_function -- rpma_log_default_function() mock
 */
void
rpma_log_default_function(enum rpma_log_level level, const char *file_name,
	const int line_no, const char *function_name,
	const char *message_format, ...)
{

}

/*
 * rpma_log_default_init -- rpma_log_default_init() mock
 */
void
rpma_log_default_init(void)
{
	function_called();
}

/*
 * rpma_log_default_fini -- rpma_log_default_fini() mock
 */
void
rpma_log_default_fini(void)
{
	function_called();
}
