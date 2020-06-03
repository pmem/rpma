/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2020, Intel Corporation
 */


#if 0
/*
 * setup_without_custom_log_function -- logging setup without user-defined
 * function - default log enabling path expected
 */
int
setup_without_custom_log_function(void **p_custom_log_function)
{
	*p_custom_log_function = NULL;
	expect_string(openlog, __ident, "rpma");
	expect_value(openlog, __option, LOG_PID);
	expect_value(openlog, __facility, LOG_LOCAL7);
	assert_int_equal(0, rpma_log_init(NULL));
	return 0;
}
#endif
#if 0


/*
 * teardown -- logging finit and closelog() called in case of no user-defined
 * log function
 */
int
teardown(void **p_custom_log_function)
{
	log_function *custom_log_function = *p_custom_log_function;
	if (NULL == custom_log_function)
		expect_function_call(closelog);
	rpma_log_fini();
	return 0;
}

/*
 * log__lifecycle -- logging lifecycle - proper sequence of in logging
 * initialization and shutdown
 */
void
log__lifecycle(void **unused)
{
	/*
	 * The whole thing is done by setup_without_custom_log_function()
	 * or setup_with_custom_log_function
	 * and teardown().
	 */
}

#endif
#if 0
	cmocka_unit_test(syslog_set_threshold),
	cmocka_unit_test(syslog_set_threshold__invalid),
	cmocka_unit_test(stderr_set_threshold),
	cmocka_unit_test(stderr_set_threshold__invalid),

	cmocka_unit_test(log__out_of_threshold),
	cmocka_unit_test(log__to_syslog),
	cmocka_unit_test(log__to_syslog_no_file),
	cmocka_unit_test(test_log__log_to_stderr),

	cmocka_unit_test(test_log__could_not_start_already_started_log),
#endif

#if 0
cmocka_unit_test_setup_teardown(log__lifecycle,
			setup_without_custom_log_function, teardown),
		cmocka_unit_test_setup_teardown(log__lifecycle,
			setup_with_custom_log_function, teardown),
		/*
		 * negative tests with custom log function
		 */
		cmocka_unit_test_setup_teardown(syslog_set_threshold,
			setup_without_custom_log_function, teardown),
		cmocka_unit_test_setup_teardown(syslog_set_threshold,
			setup_with_custom_log_function, teardown),
		cmocka_unit_test_setup_teardown(syslog_set_threshold__invalid,
			setup_without_custom_log_function, teardown),
		cmocka_unit_test_setup_teardown(syslog_set_threshold__invalid,
			setup_with_custom_log_function, teardown),
		/*
		 * negative test with default log function
		 */
		cmocka_unit_test_setup_teardown(stderr_set_threshold,
			setup_without_custom_log_function, teardown),
		cmocka_unit_test_setup_teardown(stderr_set_threshold,
			setup_with_custom_log_function, teardown),
		cmocka_unit_test_setup_teardown(stderr_set_threshold__invalid,
			setup_without_custom_log_function, teardown),
		cmocka_unit_test_setup_teardown(stderr_set_threshold__invalid,
			setup_with_custom_log_function, teardown),
		cmocka_unit_test_setup_teardown(log__to_user_function,
			setup_with_custom_log_function, teardown),
		/*
		 * negative test with level out of threshold
		 */
		cmocka_unit_test_setup_teardown(log__out_of_threshold,
			setup_without_custom_log_function, teardown),
		cmocka_unit_test_setup_teardown(log__to_syslog,
			setup_without_custom_log_function, teardown),
		cmocka_unit_test_setup_teardown(log__to_syslog_no_file,
			setup_without_custom_log_function, teardown),
	};

#endif

#if 0
	cmocka_unit_test(stderr_set_threshold),
	cmocka_unit_test(stderr_set_threshold__invalid),

	cmocka_unit_test(log__out_of_threshold),
	cmocka_unit_test(log__to_syslog),
	cmocka_unit_test(log__to_syslog_no_file),
	cmocka_unit_test(test_log__log_to_stderr),

	cmocka_unit_test(test_log__could_not_start_already_started_log),
#endif

#if 0
/*
 * setup_without_custom_log_function -- logging setup without user-defined
 * function - default log enabling path expected
 */
int
setup_without_custom_log_function(void **p_custom_log_function)
{
	*p_custom_log_function = NULL;
	expect_string(openlog, __ident, "rpma");
	expect_value(openlog, __option, LOG_PID);
	expect_value(openlog, __facility, LOG_LOCAL7);
	assert_int_equal(0, rpma_log_init(NULL));
	return 0;
}
#endif
#if 0
/*
 * setup_with_custom_log_function -- logging setup with user-defined log
 * function - no use of syslog and stderr
 */
int
setup_with_custom_log_function(void **p_custom_log_function)
{
	*p_custom_log_function = custom_log_function;
	assert_int_equal(0, rpma_log_init(custom_log_function));
	return 0;
}

/*
 * teardown -- logging finit and closelog() called in case of no user-defined
 * log function
 */
int
teardown(void **p_custom_log_function)
{
	log_function *custom_log_function = *p_custom_log_function;
	if (NULL == custom_log_function)
		expect_function_call(closelog);
	rpma_log_fini();
	return 0;
}

/*
 * log__lifecycle -- logging lifecycle - proper sequence of in logging
 * initialization and shutdown
 */
void
log__lifecycle(void **unused)
{
	/*
	 * The whole thing is done by setup_without_custom_log_function()
	 * or setup_with_custom_log_function
	 * and teardown().
	 */
}

/*
 * log__to_user_function -- logging via custom user-defined log function
 */
void
log__to_user_function(void **p_logfunction)
{
	for (rpma_log_level level = RPMA_LOG_DISABLED;
		level <= RPMA_LOG_LEVEL_DEBUG; level++) {
		expect_value(custom_log_function, level, level);
		expect_string(custom_log_function, file, TEST_FILE);
		expect_value(custom_log_function, line, 1);
		expect_string(custom_log_function, func, TEST_FUNCTION_NAME,);
		expect_string(custom_log_function, format, "%s");
		rpma_log(level, TEST_FILE_NAME, 1, TEST_FUNCTION_NAME,, "%s", TEST_MESSAGE);
	}
}
#endif
