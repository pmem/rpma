// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020-2022, Intel Corporation */

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
 * Default levels of the logging thresholds
 */
#ifdef DEBUG
#define RPMA_LOG_THRESHOLD_DEFAULT RPMA_LOG_LEVEL_DEBUG
#define RPMA_LOG_THRESHOLD_AUX_DEFAULT RPMA_LOG_LEVEL_WARNING
#else
#define RPMA_LOG_THRESHOLD_DEFAULT RPMA_LOG_LEVEL_WARNING
#define RPMA_LOG_THRESHOLD_AUX_DEFAULT RPMA_LOG_DISABLED
#endif

/*
 * Rpma_log_function -- pointer to the logging function.
 * By default it is rpma_log_default_function() but could be a user logging
 * function provided via rpma_log_set().
 */
rpma_log_function *Rpma_log_function;

/* threshold levels */
enum rpma_log_level Rpma_log_threshold[] = {
		RPMA_LOG_THRESHOLD_DEFAULT,
		RPMA_LOG_THRESHOLD_AUX_DEFAULT
};

/*
 * rpma_log_init -- initialize and set the default logging function
 */
void
rpma_log_init()
{
	/* enable the default logging function */
	rpma_log_default_init();
	while (RPMA_E_AGAIN == rpma_log_set_function(
				RPMA_LOG_USE_DEFAULT_FUNCTION))
		;
}

/*
 * rpma_log_fini -- disable logging and cleanup the default logging function
 */
void
rpma_log_fini()
{
	/*
	 * NULL-ed function pointer turns off the logging. No matter if
	 * the previous value was the default logging function or a user
	 * logging function.
	 */
	Rpma_log_function = NULL;

	/* cleanup the default logging function */
	rpma_log_default_fini();
}

/* public librpma log API */

#ifdef RPMA_UNIT_TESTS
int mock__sync_bool_compare_and_swap__function(rpma_log_function **ptr,
	rpma_log_function *oldval, rpma_log_function *newval);
#define __sync_bool_compare_and_swap \
	mock__sync_bool_compare_and_swap__function
#endif

/*
 * rpma_log_set_function -- set the log function pointer either to
 * a user-provided function pointer or to the default logging function.
 */
int
rpma_log_set_function(rpma_log_function *log_function)
{

	if (log_function == RPMA_LOG_USE_DEFAULT_FUNCTION)
		log_function = rpma_log_default_function;

	rpma_log_function *log_function_old = Rpma_log_function;

	if (__sync_bool_compare_and_swap(&Rpma_log_function,
			log_function_old, log_function))
		return 0;
	else
		return RPMA_E_AGAIN;

}

#ifdef RPMA_UNIT_TESTS
#undef __sync_bool_compare_and_swap
int mock__sync_bool_compare_and_swap__threshold(enum rpma_log_level *ptr,
	enum rpma_log_level oldval, enum rpma_log_level newval);
#define __sync_bool_compare_and_swap \
	mock__sync_bool_compare_and_swap__threshold
#endif

/*
 * rpma_log_set_threshold -- set the log level threshold
 */
int
rpma_log_set_threshold(enum rpma_log_threshold threshold,
			enum rpma_log_level level)
{
	if (threshold != RPMA_LOG_THRESHOLD &&
			threshold != RPMA_LOG_THRESHOLD_AUX)
		return RPMA_E_INVAL;

	if (level < RPMA_LOG_DISABLED || level > RPMA_LOG_LEVEL_DEBUG)
		return RPMA_E_INVAL;


	enum rpma_log_level level_old = Rpma_log_threshold[threshold];

	if (__sync_bool_compare_and_swap(&Rpma_log_threshold[threshold],
			level_old, level))
		return 0;
	else
		return RPMA_E_AGAIN;
}

#ifdef RPMA_UNIT_TESTS
#undef __sync_bool_compare_and_swap
#endif

/*
 * rpma_log_get_threshold -- get the log level threshold
 */
int
rpma_log_get_threshold(enum rpma_log_threshold threshold,
			enum rpma_log_level *level)
{
	if (threshold != RPMA_LOG_THRESHOLD &&
			threshold != RPMA_LOG_THRESHOLD_AUX)
		return RPMA_E_INVAL;

	if (level == NULL)
		return RPMA_E_INVAL;

	*level = Rpma_log_threshold[threshold];

	return 0;
}
