// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020, Intel Corporation */

/*
 * mocks-rpma-log.c -- librpma log.c module mocks
 */

#include "cmocka_headers.h"
#include "log_internal.h"

enum rpma_log_level Rpma_log_threshold[] = {
		/* all logs have to be triggered */
		RPMA_LOG_LEVEL_DEBUG,	/* RPMA_LOG_THRESHOLD */
		RPMA_LOG_DISABLED	/* RPMA_LOG_THRESHOLD_AUX */
};

/*
 * mock_function -- logging function's mock
 */
static void
mock_function(enum rpma_log_level level, const char *file_name,
	const int line_no, const char *function_name,
	const char *message_format, ...)
{
}

log_function *Rpma_log_function = mock_function;

/*
 * rpma_log_init -- rpma_log_init() mock
 */
void
rpma_log_init()
{
	function_called();
}

/*
 * rpma_log_fini -- rpma_log_fini() mock
 */
void
rpma_log_fini()
{
	function_called();
}
