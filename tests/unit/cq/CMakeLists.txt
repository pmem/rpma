#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2022, Intel Corporation
# Copyright 2021-2022, Fujitsu
#

include(../../cmake/ctest_helpers.cmake)

function (add_test_cq name)
	set(src_name cq-${name})
	set(name ut-${src_name})
	build_test_src(UNIT NAME ${name} SRCS
		${src_name}.c
		cq-common.c
		${TEST_UNIT_COMMON_DIR}/mocks-ibverbs.c
		${TEST_UNIT_COMMON_DIR}/mocks-stdlib.c
		${TEST_UNIT_COMMON_DIR}/mocks-rpma-log.c
		${LIBRPMA_SOURCE_DIR}/cq.c)

	target_compile_definitions(${name} PRIVATE TEST_MOCK_ALLOC)

	set_target_properties(${name}
		PROPERTIES
		LINK_FLAGS "-Wl,--wrap=_test_malloc")

	add_test_generic(NAME ${name} TRACERS none)
endfunction()

add_test_cq(get_fd)
add_test_cq(get_ibv_cq)
add_test_cq(get_wc)
add_test_cq(new_delete)
add_test_cq(wait)
