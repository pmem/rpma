#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2020-2022, Intel Corporation
#

include(../../cmake/ctest_helpers.cmake)

build_test_src(UNIT NAME ut-librpma_constructor SRCS
	librpma_constructor.c
	${TEST_UNIT_COMMON_DIR}/mocks-rpma-log.c
	${LIBRPMA_SOURCE_DIR}/librpma.c)

target_compile_definitions(ut-librpma_constructor PRIVATE MOCK_CONSTRUCTOR)

add_test_generic(NAME ut-librpma_constructor TRACERS none)
