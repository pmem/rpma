// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020, Intel Corporation */

/*
 * log-example.c -- an example how to use and control the log behavior
 */

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#include "librpma_log.h"

extern void log_worker_is_doing_something(void);

static void user_logfunc(int level, const char *file, const int line,
		const char *func, const char *format, va_list args)
{

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
	rpma_log_init(NULL);
	fprintf(stderr, "Let's write messages to stderr and syslog\n");
	rpma_log_stderr_set_level(RPMA_LOG_DEBUG);
	rpma_log_set_level(RPMA_LOG_DEBUG);
	log_worker_is_doing_something();
	rpma_log_fini();

	/*
	 * log messages to be transfered only to custom user function
	 */
	rpma_log_init(user_logfunc);
	fprintf(stderr, "Let's use custom log function" \
			"to write messages to stderr\n");
	fprintf(stderr, "No message should be written to syslog\n");
	log_worker_is_doing_something();
	rpma_log_fini();
	return 0;
}
