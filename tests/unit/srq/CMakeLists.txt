#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2022, Fujitsu
#

include(../../cmake/ctest_helpers.cmake)

function(add_test_srq name)
	set(src_name srq-${name})
	set(name ut-${src_name})
	build_test_src(UNIT NAME ${name} SRCS
		${src_name}.c
		srq-common.c
		${TEST_UNIT_COMMON_DIR}/mocks-stdlib.c
		${TEST_UNIT_COMMON_DIR}/mocks-ibverbs.c
		${TEST_UNIT_COMMON_DIR}/mocks-rpma-cq.c
		${TEST_UNIT_COMMON_DIR}/mocks-rpma-log.c
		${TEST_UNIT_COMMON_DIR}/mocks-rpma-mr.c
		${TEST_UNIT_COMMON_DIR}/mocks-rpma-peer.c
		${TEST_UNIT_COMMON_DIR}/mocks-rpma-srq_cfg.c
		${LIBRPMA_SOURCE_DIR}/srq.c)

	target_compile_definitions(${name} PRIVATE TEST_MOCK_ALLOC)

	set_target_properties(${name}
		PROPERTIES
		LINK_FLAGS "-Wl,--wrap=_test_malloc")

	add_test_generic(NAME ${name} TRACERS none)
endfunction()

add_test_srq(get_ibv_srq)
add_test_srq(get_rcq)
add_test_srq(new_delete)
add_test_srq(recv)
