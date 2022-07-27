// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020-2022, Intel Corporation */

/*
 * mocks-rpma-log.c -- librpma log.c module mocks
 */

#include "cmocka_headers.h"
#include "log_internal.h"

#ifdef ATOMIC_OPERATIONS_SUPPORTED
_Atomic
#endif /* ATOMIC_OPERATIONS_SUPPORTED */
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

#ifdef ATOMIC_OPERATIONS_SUPPORTED
_Atomic
#endif /* ATOMIC_OPERATIONS_SUPPORTED */
uintptr_t Rpma_log_function = (uintptr_t)mock_function;

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

/*
 * rpma_log_fini -- rpma_log_get_threshold() mock
 */
int
rpma_log_get_threshold(enum rpma_log_threshold threshold,
				enum rpma_log_level *level)
{
	if (level == NULL)
		return RPMA_E_INVAL;

	*level = RPMA_LOG_LEVEL_NOTICE;

	return 0;
}
