#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2020, Intel Corporation
#

include(../cmake/ctest_helpers.cmake)

function(add_conn_test test)
	set(name conn-test-${test})
	build_test_src(${name}
		${name}.c
		conn-test-common.c
	       ${LIBRPMA_SOURCE_DIR}/rpma_err.c
		${LIBRPMA_SOURCE_DIR}/conn.c)

	target_compile_definitions(${name} PUBLIC TEST_MOCK_ALLOC)

	set_target_properties(${name}
		PROPERTIES
		LINK_FLAGS "-Wl,--wrap=_test_malloc")

	add_test_generic(NAME ${name} TRACERS none)
endfunction()

add_conn_test(new)
add_conn_test(next-event)
add_conn_test(disconnect)
add_conn_test(private-data)
add_conn_test(read)
add_conn_test(next-completion)
