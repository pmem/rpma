#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2020, Intel Corporation
#

include(../../cmake/ctest_helpers.cmake)

build_test_src(UNIT NAME flush-test SRCS
	flush-test.c
	${LIBRPMA_SOURCE_DIR}/rpma_err.c
	${LIBRPMA_SOURCE_DIR}/flush.c
	${TEST_UNIT_COMMON_DIR}/mocks-ibverbs.c
	${TEST_UNIT_COMMON_DIR}/mocks-rpma-mr.c)

target_compile_definitions(flush-test PRIVATE TEST_MOCK_ALLOC)

set_target_properties(flush-test
	PROPERTIES
	LINK_FLAGS "-Wl,--wrap=_test_malloc,--wrap=posix_memalign")

add_test_generic(NAME flush-test TRACERS none)
