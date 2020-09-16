#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2020, Intel Corporation
#

include(../../cmake/ctest_helpers.cmake)

function(add_test_info name)
	set(name info-${name})
	build_test_src(UNIT NAME ${name} SRCS
		${name}.c
		info-common.c
		${TEST_UNIT_COMMON_DIR}/mocks-ibverbs.c
		${TEST_UNIT_COMMON_DIR}/mocks-rdma_cm.c
		${TEST_UNIT_COMMON_DIR}/mocks-rpma-log.c
		${TEST_UNIT_COMMON_DIR}/mocks-stdlib.c
		${LIBRPMA_SOURCE_DIR}/rpma_err.c
		${LIBRPMA_SOURCE_DIR}/info.c)
	
	if(NOT SIGNATURE_OK_RDMA_GETADDRINFO)
		target_compile_definitions(${name} PRIVATE OLD_SIGNATURE_RDMA_GETADDRINFO=1)
	endif()

	target_compile_definitions(${name} PRIVATE TEST_MOCK_ALLOC)

	set_target_properties(${name}
		PROPERTIES
		LINK_FLAGS "-Wl,--wrap=_test_malloc")

	add_test_generic(NAME ${name} TRACERS none)
endfunction()

add_test_info(new)
add_test_info(bind_addr)
add_test_info(resolve_addr)
