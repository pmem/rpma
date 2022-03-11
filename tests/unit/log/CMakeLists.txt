#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2020-2022, Intel Corporation
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
		${LIBRPMA_SOURCE_DIR}/log.c)

	target_compile_definitions(${test} PRIVATE RPMA_UNIT_TESTS)

	if ("${ARGV1}" STREQUAL "DEBUG")
		target_compile_definitions(${test} PRIVATE DEBUG)
	endif()

	add_test_generic(NAME ${test} TRACERS none)
endfunction()

add_test_log(init-fini)
add_test_log(init-fini DEBUG)
add_test_log(macros)
add_test_log(threshold)
