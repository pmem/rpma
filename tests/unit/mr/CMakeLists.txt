#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2020-2021, Intel Corporation
#

include(../../cmake/ctest_helpers.cmake)

function(add_test_mr name)
	set(name mr-${name})
	build_test_src(UNIT NAME ${name} SRCS
		${name}.c
		mr-common.c
		${TEST_UNIT_COMMON_DIR}/mocks-ibverbs.c
		${TEST_UNIT_COMMON_DIR}/mocks-rpma-log.c
		${TEST_UNIT_COMMON_DIR}/mocks-rpma-peer.c
		${TEST_UNIT_COMMON_DIR}/mocks-stdlib.c
		${LIBRPMA_SOURCE_DIR}/rpma_err.c
		${LIBRPMA_SOURCE_DIR}/mr.c)

	target_compile_definitions(${name} PRIVATE TEST_MOCK_ALLOC)

	set_target_properties(${name}
		PROPERTIES
		LINK_FLAGS "-Wl,--wrap=_test_malloc")

	add_test_generic(NAME ${name} TRACERS none)
endfunction()

add_test_mr(advise)
add_test_mr(descriptor)
add_test_mr(get_flush_type)
add_test_mr(local)
add_test_mr(read)
add_test_mr(recv)
add_test_mr(reg)
add_test_mr(send)
add_test_mr(write)
