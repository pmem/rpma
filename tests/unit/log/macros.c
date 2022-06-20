// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020-2022, Intel Corporation */

/*
 * macros.c -- RPMA_LOG_* macros unit tests
 */

#include "cmocka_headers.h"
#include "log_internal.h"
#include "log_default.h"

#define MOCK_MESSAGE	"Message"
#define MOCK_OUTPUT	1024
#define MOCK_FILE_NAME	__FILE__

/*
 * mock_log_function -- custom log function
 */
void
mock_log_function(enum rpma_log_level level, const char *file_name,
		const int line_no, const char *function_name,
		const char *message_format, ...)
{
	static char output[MOCK_OUTPUT];

	check_expected(level);
	check_expected(file_name);
	check_expected(line_no);
	check_expected(function_name);

	va_list ap;
	va_start(ap, message_format);
	assert_true(vsnprintf(output, MOCK_OUTPUT, message_format, ap) > 0);
	va_end(ap);

	check_expected_ptr(output);
}

/*
 * setup_threshold -- set the primary threshold
 */
int
setup_threshold(void **level_ptr)
{
	enum rpma_log_level level = **(enum rpma_log_level **)level_ptr;
	while (RPMA_E_AGAIN == rpma_log_set_threshold(
				RPMA_LOG_THRESHOLD, level))
		;
	return 0;
}

/*
 * MOCK_CONFIGURE_LOG_FUNC -- configure the set of expects needed for
 * the mock_log_function function
 *
 */
#define MOCK_CONFIGURE_LOG_FUNC(l) \
	expect_value(mock_log_function, level, (l)); \
	expect_string(mock_log_function, file_name, MOCK_FILE_NAME); \
	expect_value(mock_log_function, line_no, __LINE__ + 2); \
	expect_string(mock_log_function, function_name, "log__all"); \
	expect_string(mock_log_function, output, MOCK_MESSAGE "\n") \

/*
 * log__all -- happy day scenario
 */
static void
log__all(void **level_ptr)
{
	enum rpma_log_level primary = **(enum rpma_log_level **)level_ptr;

	for (enum rpma_log_level secondary = RPMA_LOG_DISABLED;
			secondary <= RPMA_LOG_LEVEL_DEBUG; ++secondary) {

		/*
		 * The secondary threshold should not affect the macros
		 * behaviour.
		 */
		while (RPMA_E_AGAIN == rpma_log_set_threshold(
					RPMA_LOG_THRESHOLD_AUX, secondary))
			;

		if (RPMA_LOG_LEVEL_NOTICE <= primary) {
			MOCK_CONFIGURE_LOG_FUNC(RPMA_LOG_LEVEL_NOTICE);
		}
		RPMA_LOG_NOTICE("%s", MOCK_MESSAGE);

		if (RPMA_LOG_LEVEL_WARNING <= primary) {
			MOCK_CONFIGURE_LOG_FUNC(RPMA_LOG_LEVEL_WARNING);
		}
		RPMA_LOG_WARNING("%s", MOCK_MESSAGE);

		if (RPMA_LOG_LEVEL_ERROR <= primary) {
			MOCK_CONFIGURE_LOG_FUNC(RPMA_LOG_LEVEL_ERROR);
		}
		RPMA_LOG_ERROR("%s", MOCK_MESSAGE);

		if (RPMA_LOG_LEVEL_FATAL <= primary) {
			MOCK_CONFIGURE_LOG_FUNC(RPMA_LOG_LEVEL_FATAL);
		}
		RPMA_LOG_FATAL("%s", MOCK_MESSAGE);

		/* RPMA_LOG_ALWAYS() has to always call rpma_log_default_function() */
		expect_function_call(rpma_log_default_function);
		RPMA_LOG_ALWAYS("%s", MOCK_MESSAGE);
	}
}

int
main(int argc, char *argv[])
{
	/* set a custom logging function */
	while (RPMA_E_AGAIN == rpma_log_set_function(
				mock_log_function))
		;

	/* prestates */
	enum rpma_log_level Level_disabled = RPMA_LOG_DISABLED;
	enum rpma_log_level Level_fatal = RPMA_LOG_LEVEL_FATAL;
	enum rpma_log_level Level_error = RPMA_LOG_LEVEL_ERROR;
	enum rpma_log_level Level_warning = RPMA_LOG_LEVEL_WARNING;
	enum rpma_log_level Level_notice = RPMA_LOG_LEVEL_NOTICE;
	enum rpma_log_level Level_info = RPMA_LOG_LEVEL_INFO;
	enum rpma_log_level Level_debug = RPMA_LOG_LEVEL_DEBUG;

	const struct CMUnitTest tests[] = {
		{"RPMA_LOG_DISABLED", log__all, setup_threshold, NULL,
				&Level_disabled},
		{"RPMA_LOG_LEVEL_FATAL", log__all, setup_threshold, NULL,
				&Level_fatal},
		{"RPMA_LOG_LEVEL_ERROR", log__all, setup_threshold, NULL,
				&Level_error},
		{"RPMA_LOG_LEVEL_WARNING", log__all, setup_threshold, NULL,
				&Level_warning},
		{"RPMA_LOG_LEVEL_NOTICE", log__all, setup_threshold, NULL,
				&Level_notice},
		{"RPMA_LOG_LEVEL_INFO", log__all, setup_threshold, NULL,
				&Level_info},
		{"RPMA_LOG_LEVEL_DEBUG", log__all, setup_threshold, NULL,
				&Level_debug},
	};

	return cmocka_run_group_tests(tests, NULL, NULL);
}
