// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020, Intel Corporation */

/*
 * log.c -- support for logging output to either syslog or stderr or
 * via user defined function
 */

#include <stdarg.h>
#include <syslog.h>
#include <time.h>
#include <string.h>

#include "librpma.h"
#include "log_default.h"
#include "log_internal.h"

/*
 * Rpma_log_function -- pointer to the logging function.
 * By default it is rpma_log_default_function() but could be a user logging
 * function provided via rpma_log_set().
 */
log_function *Rpma_log_function;

/* threshold levels */
rpma_log_level Rpma_log_threshold[] = {
		RPMA_LOG_THRESHOLD_PRIMARY_DEFAULT,
		RPMA_LOG_THRESHOLD_SECONDARY_DEFAULT
};

/*
 * rpma_log_init -- initialize and set the default logging function
 */
void
rpma_log_init()
{
	/* enable the default logging function */
	rpma_log_default_init();
	rpma_log_set_function(RPMA_LOG_DEFAULT_FUNCTION);
}

/*
 * rpma_log_fini -- disable logging and cleanup the default logging function
 */
void
rpma_log_fini()
{
	Rpma_log_function = NULL;

	/* cleanup the default logging function */
	rpma_log_default_fini();
}

/* public librpma log API */

/*
 * rpma_log_set_function -- set the log function pointer either to
 * a user-provided function pointer or to the default logging function.
 */
void
rpma_log_set_function(log_function *log_function)
{
	if (log_function == RPMA_LOG_DEFAULT_FUNCTION)
		Rpma_log_function = rpma_log_default_function;
	else
		Rpma_log_function = log_function;
}

/*
 * rpma_log_set_threshold -- set the log level threshold
 */
int
rpma_log_set_threshold(rpma_threshold threshold, rpma_log_level level)
{
	if (threshold != RPMA_LOG_THRESHOLD_PRIMARY &&
			threshold != RPMA_LOG_THRESHOLD_SECONDARY)
		return RPMA_E_INVAL;

	if (level < RPMA_LOG_DISABLED || level > RPMA_LOG_LEVEL_DEBUG)
		return RPMA_E_INVAL;

	Rpma_log_threshold[threshold] = level;

	return 0;
}

/*
 * rpma_log_get_threshold -- get the log level threshold
 */
int
rpma_log_get_threshold(rpma_threshold threshold, rpma_log_level *level)
{
	if (threshold != RPMA_LOG_THRESHOLD_PRIMARY &&
			threshold != RPMA_LOG_THRESHOLD_SECONDARY)
		return RPMA_E_INVAL;

	if (level == NULL)
		return RPMA_E_INVAL;

	*level = Rpma_log_threshold[threshold];

	return 0;
}
