#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2020, Intel Corporation
#

include(../../cmake/ctest_helpers.cmake)

function(add_test_log name)
	build_test_src(UNIT NAME log-${name} SRCS
		${name}.c
		${TEST_UNIT_COMMON_DIR}/mocks-rpma-log_default.c
		${LIBRPMA_SOURCE_DIR}/log.c)

	add_test_generic(NAME log-${name} TRACERS none)
endfunction()

add_test_log(init-fini)
add_test_log(macros)

# init-fini with DEBUG
build_test_src(UNIT NAME log-init-fini-DEBUG SRCS
	init-fini.c
	${TEST_UNIT_COMMON_DIR}/mocks-rpma-log_default.c
	${LIBRPMA_SOURCE_DIR}/log.c)
target_compile_definitions(log-init-fini-DEBUG PRIVATE DEBUG)

add_test_generic(NAME log-init-fini-DEBUG TRACERS none)