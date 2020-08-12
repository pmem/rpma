#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2020, Intel Corporation
#

include(../../cmake/ctest_helpers.cmake)

function(build_log name)
	build_test_src(UNIT NAME log-${name} SRCS
		${name}.c
		${TEST_UNIT_COMMON_DIR}/mocks-rpma-log_default.c
		${LIBRPMA_SOURCE_DIR}/log.c)

	add_test_generic(NAME log-${name} TRACERS none)
endfunction()

build_log(init-fini)
build_log(macros)