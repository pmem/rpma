#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2020, Intel Corporation
#

include(../../cmake/ctest_helpers.cmake)

build_test_src(UNIT NAME mr-test SRCS
	mr-test.c
	mr-test-common.c
	mr-test-descriptor.c
	mr-test-read.c
	mr-test-reg.c
	mr-test-write.c
	${TEST_UNIT_COMMON_DIR}/mocks-rpma-log.c
	${LIBRPMA_SOURCE_DIR}/rpma_err.c
	${LIBRPMA_SOURCE_DIR}/mr.c)

target_compile_definitions(mr-test PRIVATE TEST_MOCK_ALLOC)

set_target_properties(mr-test
	PROPERTIES
	LINK_FLAGS "-Wl,--wrap=_test_malloc")

add_test_generic(NAME mr-test TRACERS none)
