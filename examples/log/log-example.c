// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020, Intel Corporation */

/*
 * log-example.c -- an example of how to use and control the log behavior
 */

#include <stdio.h>
#include "librpma_log.h"

extern void log_worker_is_doing_something(void);

static void
user_log_function(int level, const char *file, const int line,
		const char *func, const char *format, va_list args)
{
	if (((NULL != file) && (NULL==func)) || (NULL == format)) {
		return;
	}
	fprintf(stderr, "Custom log handling: \n");
	if (NULL != file) {
		fprintf(stderr, "%s %4d %s:\n", file, line, func);
	}
	fprintf(stderr, "level: %d ", level);
	vfprintf(stderr, format, args);
	if (NULL != file) {
		fprintf(stderr, "\n");
	}
}

int
main(int argc, char *argv[])
{
	/*
	 * log messages to be produced to syslog as well as stderr
	 */
	printf("Let's write messages to stderr and syslog\n");
	rpma_log_stderr_set_level(RPMA_LOG_LEVEL_DEBUG);
	rpma_log_set_level(RPMA_LOG_LEVEL_DEBUG);
	log_worker_is_doing_something();
	rpma_log_fini();

	/*
	 * log messages to be transfered only to custom user function
	 */
	if (rpma_log_init(user_log_function)) {
		fprintf(stderr, "Could not initialize log\n");
		return -1;
	}

	printf("Let's use custom log function to write messages to stderr\n" \
		"No message should be written to syslog\n");
	log_worker_is_doing_something();
	rpma_log_fini();

	return 0;
}
