// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020, Intel Corporation */

/*
 * log-example.c -- an example of how to use and control the log behavior
 */

#include <stdio.h>
#include "librpma_log.h"

extern void log_worker_is_doing_something(void);

static void
user_log_function(int level, const char *file_name, const int line_no,
		const char *func_name, const char *message_format,
		va_list args)
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
	if (vfprintf(stderr, message_format, args) < 0)
		return;
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
	rpma_log_stderr_set_threshold(RPMA_LOG_LEVEL_DEBUG);
	rpma_log_syslog_set_threshold(RPMA_LOG_LEVEL_DEBUG);
	log_worker_is_doing_something();
	rpma_log_fini();
	printf(
		"Use: \n$ sudo tail -n 60 /var/log/syslog | grep rpma\nto see messages in the syslog.");

	/*
	 * log messages to be transfered only to custom user function
	 */
	if (rpma_log_init(user_log_function)) {
		(void) fprintf(stderr, "Could not initialize log\n");
		return -1;
	}

	printf(
		"Let's use custom log function to write messages to stderr\nNo message should be written to syslog\n");
	log_worker_is_doing_something();
	rpma_log_fini();

	return 0;
}
