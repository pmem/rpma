#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2020, Intel Corporation
#

include(../../cmake/ctest_helpers.cmake)

build_test_src(UNIT NAME conn-test SRCS
	conn-test.c
	conn-test-common.c
	conn-test-disconnect.c
	conn-test-get_completion_fd.c
	conn-test-get_event_fd.c
	conn-test-new.c
	conn-test-next-completion.c
	conn-test-next-event.c
	conn-test-private-data.c
	conn-test-read.c
	conn-test-write.c
	${TEST_UNIT_COMMON_DIR}/mocks-ibverbs.c
	${TEST_UNIT_COMMON_DIR}/mocks-rpma-flush.c
	${LIBRPMA_SOURCE_DIR}/rpma_err.c
	${LIBRPMA_SOURCE_DIR}/conn.c)

target_compile_definitions(conn-test PRIVATE TEST_MOCK_ALLOC)

set_target_properties(conn-test
	PROPERTIES
	LINK_FLAGS "-Wl,--wrap=_test_malloc")

add_test_generic(NAME conn-test TRACERS none)
