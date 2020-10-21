// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020, Intel Corporation */

/*
 * log-example.c -- an example of how to use and control behavior of the log.
 */

#include <stdarg.h>
#include <stdio.h>

#include "librpma.h"

extern void log_worker_is_doing_something(void);

static void
user_log_function(int level, const char *file_name, const int line_no,
		const char *func_name, const char *message_format, ...)
{
	if (((NULL != file_name) && (NULL == func_name)) ||
			(NULL == message_format)) {
		return;
	}
	if (fprintf(stderr, "Custom log handling: \n") < 0)
		return;
	if (NULL != file_name) {
		if (fprintf(stderr, "%s %4d %s:\n", file_name, line_no,
				func_name) < 0) {
			return;
		}
	}

	if (fprintf(stderr, "level: %d ", level) < 0)
		return;

	va_list args;
	va_start(args, message_format);
	if (vfprintf(stderr, message_format, args) < 0) {
		va_end(args);
		return;
	}
	va_end(args);

	if (NULL != file_name)
		fprintf(stderr, "\n");
}

int
main(int argc, char *argv[])
{
	/*
	 * log messages to be produced to syslog as well as stderr
	 */
	printf("Let's write messages to stderr and syslog\n");
	rpma_log_set_threshold(RPMA_LOG_THRESHOLD,
			RPMA_LOG_LEVEL_DEBUG);
	rpma_log_set_threshold(RPMA_LOG_THRESHOLD_AUX,
			RPMA_LOG_LEVEL_DEBUG);
	log_worker_is_doing_something();
	printf(
		"Use: \n$ sudo tail -n 60 /var/log/syslog | grep rpma\nto see messages in the syslog.");

	/*
	 * log messages to be transferred only to custom user function
	 */
	rpma_log_set_function(user_log_function);
	printf(
		"Let's use custom log function to write messages to stderr\nNo message should be written to syslog\n");
	log_worker_is_doing_something();
	rpma_log_set_function(RPMA_LOG_USE_DEFAULT_FUNCTION);

	return 0;
}
