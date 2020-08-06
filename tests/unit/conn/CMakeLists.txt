#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2020, Intel Corporation
#

include(../../cmake/ctest_helpers.cmake)

build_test_src(UNIT NAME conn SRCS
	conn.c
	conn-common.c
	conn-disconnect.c
	conn-flush.c
	conn-get_completion_fd.c
	conn-get_event_fd.c
	conn-new.c
	conn-next-completion.c
	conn-next-event.c
	conn-private-data.c
	conn-read.c
	conn-write.c
	${TEST_UNIT_COMMON_DIR}/mocks-ibverbs.c
	${TEST_UNIT_COMMON_DIR}/mocks-rpma-flush.c
	${LIBRPMA_SOURCE_DIR}/rpma_err.c
	${LIBRPMA_SOURCE_DIR}/conn.c)

target_compile_definitions(conn PRIVATE TEST_MOCK_ALLOC)

set_target_properties(conn
	PROPERTIES
	LINK_FLAGS "-Wl,--wrap=_test_malloc")

add_test_generic(NAME conn TRACERS none)
