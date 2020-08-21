#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2020, Intel Corporation
#

include(../../cmake/ctest_helpers.cmake)

build_test_src(UNIT NAME constructor SRCS
	constructor.c
	${TEST_UNIT_COMMON_DIR}/mocks-rpma-log.c
	${LIBRPMA_SOURCE_DIR}/librpma.c)

target_compile_definitions(constructor PRIVATE MOCK_CONSTRUCTOR)

add_test_generic(NAME constructor TRACERS none)
