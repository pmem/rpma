#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2020, Intel Corporation
#

include(../../cmake/ctest_helpers.cmake)

function(add_test_log name)
	if ("${ARGV1}" STREQUAL "DEBUG")
		set(test log-${name}-DEBUG)
	else()
		set(test log-${name})
	endif()

	build_test_src(UNIT NAME ${test} SRCS
		${name}.c
		${TEST_UNIT_COMMON_DIR}/mocks-rpma-log_default.c
		${LIBRPMA_SOURCE_DIR}/log.c
		${TEST_UNIT_COMMON_DIR}/mocks-compare_and_swap.c)

	if ("${ARGV1}" STREQUAL "DEBUG")
		target_compile_definitions(${test} PRIVATE DEBUG)
	endif()
	
	if(("${name}" STREQUAL "threshold") OR ("${name}" STREQUAL "set_log_function"))
		target_compile_definitions(${test} PRIVATE TEST_MOCK_COMPARE_AND_SWAP)
	endif()

	set_target_properties(${test} PROPERTIES
		LINK_FLAGS
		"-Wl,--wrap=sync_bool_compare_and_swap_void,--wrap=sync_bool_compare_and_swap_int")

	add_test_generic(NAME ${test} TRACERS none)
endfunction()

add_test_log(init-fini)
add_test_log(init-fini DEBUG)
add_test_log(macros)
add_test_log(threshold)
add_test_log(set_log_function)
