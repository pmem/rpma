// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2019-2020, Intel Corporation */

#include <stddef.h>
#include <stdarg.h>
#include <syslog.h>
#include <time.h>
#include <string.h>
#include <ctype.h>
#ifndef assert
#include <assert.h>
#endif

#include "librpma_log_internal.h"

#ifdef RPMA_LOG_BACKTRACE_LVL
#define UNW_LOCAL_ONLY
#include <libunwind.h>
#endif

static const char *const rpma_level_names[] = {
	[RPMA_LOG_ERROR]	= "ERROR",
	[RPMA_LOG_WARN]		= "WARNING",
	[RPMA_LOG_NOTICE]	= "NOTICE",
	[RPMA_LOG_INFO]		= "INFO",
	[RPMA_LOG_DEBUG]	= "DEBUG",
};

#define MAX_TMPBUF 1024

static logfunc *g_log = NULL;

enum rpma_log_level g_rpma_log_level = RPMA_LOG_NOTICE;
enum rpma_log_level g_rpma_log_print_level = RPMA_LOG_NOTICE;
enum rpma_log_level g_rpma_log_backtrace_level = RPMA_LOG_DISABLED;

void
rpma_log_open(logfunc *logf)
{
	if (logf) {
		g_log = logf;
	} else {
		openlog("rpma", LOG_PID, LOG_LOCAL7);
	}
}

void
rpma_log_close(void)
{
	if (!g_log) {
		closelog();
	} else {
		g_log = NULL;
	}
}

#ifdef RPMA_LOG_BACKTRACE_LVL
static void
log_unwind_stack(FILE *fp, enum rpma_log_level level)
{
	unw_error_t err;
	unw_cursor_t cursor;
	unw_context_t uc;
	unw_word_t ip;
	unw_word_t offp;
	char f_name[64];
	int frame;

	if (level > g_rpma_log_backtrace_level) {
		return;
	}

	unw_getcontext(&uc);
	unw_init_local(&cursor, &uc);
	fprintf(fp, "*%s*: === BACKTRACE START ===\n", rpma_level_names[level]);

	unw_step(&cursor);
	for (frame = 1; unw_step(&cursor) > 0; frame++) {
		unw_get_reg(&cursor, UNW_REG_IP, &ip);
		err = unw_get_proc_name(&cursor, f_name, sizeof(f_name), &offp);
		if (err || strcmp(f_name, "main") == 0) {
			break;
		}

		fprintf(fp, "*%s*: %3d: %*s%s() at %#lx\n", \
				rpma_level_names[level], frame, frame - 1, "", \
				f_name, (unsigned long)ip);
	}
	fprintf(fp, "*%s*: === BACKTRACE END ===\n", rpma_level_names[level]);
}

#else
#define log_unwind_stack(fp, lvl)
#endif

static void
get_timestamp_prefix(char *buf, size_t buf_size)
{
	struct tm *info;
	char date[24];
	struct timespec ts;
	long usec;

	clock_gettime(CLOCK_REALTIME, &ts);
	info = localtime(&ts.tv_sec);
	usec = ts.tv_nsec / 1000;

	strftime(date, sizeof(date), "%Y-%m-%d %H:%M:%S", info);
	snprintf(buf, buf_size, "[%s.%06ld] ", date, usec);
}

void
rpma_log(enum rpma_log_level level, const char *file, const int line, \
		const char *func, const char *format, ...)
{
	va_list ap;

	va_start(ap, format);
	rpma_vlog(level, file, line, func, format, ap);
	va_end(ap);
}

void
rpma_vlog(enum rpma_log_level level, const char *file, const int line, \
		const char *func, const char *format, va_list ap)
{
	int severity = LOG_INFO;
	char prefix[256];
	char timestamp[45];
	char message[MAX_TMPBUF];

	if (g_log) {
		g_log(level, file, line, func, format, ap);
		return;
	}

	if (level > g_rpma_log_print_level && level > g_rpma_log_level) {
		return;
	}

	switch (level) {
	case RPMA_LOG_ERROR:
		severity = LOG_ERR;
		break;
	case RPMA_LOG_WARN:
		severity = LOG_WARNING;
		break;
	case RPMA_LOG_NOTICE:
		severity = LOG_NOTICE;
		break;
	case RPMA_LOG_INFO:
		severity = LOG_INFO;
		break;
	case RPMA_LOG_DEBUG:
		severity = LOG_DEBUG;
		break;
	case RPMA_LOG_DISABLED:
		return;
	}

	vsnprintf(message, sizeof(message), format, ap);

	if (file) {
		snprintf(prefix, sizeof(prefix), \
				"%s:%4d:%s: *%s*: ", \
				file, line, func, rpma_level_names[level]);
	} else {
		prefix[0] = '\0';
	}
	if (level <= g_rpma_log_print_level) {
		get_timestamp_prefix(timestamp, sizeof(timestamp));
		fprintf(stderr, "%s%s%s", timestamp, prefix, message);
		if (file) {
			log_unwind_stack(stderr, level);
		}
	}
	if (level <= g_rpma_log_level) {
		syslog(severity, "%s%s", prefix, message);
	}
}

void
rpma_log_set_level(enum rpma_log_level level)
{
	assert(level >= RPMA_LOG_DISABLED);
	assert(level <= RPMA_LOG_DEBUG);
	g_rpma_log_level = level;
}

enum rpma_log_level
rpma_log_get_level(void)
{
	return g_rpma_log_level;
}

void
rpma_log_set_print_level(enum rpma_log_level level)
{
	assert(level >= RPMA_LOG_DISABLED);
	assert(level <= RPMA_LOG_DEBUG);
	g_rpma_log_print_level = level;
}

enum rpma_log_level
rpma_log_get_print_level(void)
{
	return g_rpma_log_print_level;
}

void
rpma_log_set_backtrace_level(enum rpma_log_level level)
{
	assert(level >= RPMA_LOG_DISABLED);
	assert(level <= RPMA_LOG_DEBUG);
	g_rpma_log_backtrace_level = level;
}

enum rpma_log_level
rpma_log_get_backtrace_level(void)
{
	return g_rpma_log_backtrace_level;
}
