#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2020, Intel Corporation
#

include(../../cmake/ctest_helpers.cmake)

function(add_test_conn name)
	build_test_src(UNIT NAME ${name} SRCS
		${name}.c
		conn-common.c
		${TEST_UNIT_COMMON_DIR}/mocks-ibverbs.c
		${TEST_UNIT_COMMON_DIR}/mocks-rdma_cm.c
		${TEST_UNIT_COMMON_DIR}/mocks-rpma-flush.c
		${TEST_UNIT_COMMON_DIR}/mocks-rpma-mr.c
		${TEST_UNIT_COMMON_DIR}/mocks-stdlib.c
		${LIBRPMA_SOURCE_DIR}/rpma_err.c
		${LIBRPMA_SOURCE_DIR}/conn.c)

	target_compile_definitions(${name} PRIVATE TEST_MOCK_ALLOC)

	set_target_properties(${name}
		PROPERTIES
		LINK_FLAGS "-Wl,--wrap=_test_malloc")

	add_test_generic(NAME ${name} TRACERS none)
endfunction()

add_test_conn(conn-disconnect)
add_test_conn(conn-flush)
add_test_conn(conn-get_completion_fd)
add_test_conn(conn-get_event_fd)
add_test_conn(conn-new)
add_test_conn(conn-next-completion)
add_test_conn(conn-next-event)
add_test_conn(conn-prepare-completions)
add_test_conn(conn-private-data)
add_test_conn(conn-read)
add_test_conn(conn-write)
