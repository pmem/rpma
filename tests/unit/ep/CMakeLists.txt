#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2020, Intel Corporation
#

include(../../cmake/ctest_helpers.cmake)

function(add_test_ep name)
	set(name ep-${name})
	build_test_src(UNIT NAME ${name} SRCS
		${name}.c
		ep-common.c
		${TEST_UNIT_COMMON_DIR}/mocks-rpma-conn_cfg.c
		${TEST_UNIT_COMMON_DIR}/mocks-rpma-log.c
		${TEST_UNIT_COMMON_DIR}/mocks-stdlib.c
		${LIBRPMA_SOURCE_DIR}/rpma_err.c
		${LIBRPMA_SOURCE_DIR}/ep.c)

	target_compile_definitions(${name} PRIVATE TEST_MOCK_ALLOC)

	set_target_properties(${name}
		PROPERTIES
		LINK_FLAGS "-Wl,--wrap=_test_malloc")

	add_test_generic(NAME ${name} TRACERS none)
endfunction()

add_test_ep(listen)
add_test_ep(next_conn_req)
add_test_ep(get_fd)
