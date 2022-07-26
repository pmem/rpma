#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2020-2022, Intel Corporation
# Copyright 2021-2022, Fujitsu
#

include(../../cmake/ctest_helpers.cmake)

function(add_test_peer name)
	set(src_name peer-${name})
	set(name ut-${src_name})
	build_test_src(UNIT NAME ${name} SRCS
		${src_name}.c
		peer-common.c
		${TEST_UNIT_COMMON_DIR}/mocks-ibverbs.c
		${TEST_UNIT_COMMON_DIR}/mocks-rdma_cm.c
		${TEST_UNIT_COMMON_DIR}/mocks-rpma-conn_cfg.c
		${TEST_UNIT_COMMON_DIR}/mocks-rpma-cq.c
		${TEST_UNIT_COMMON_DIR}/mocks-rpma-log.c
		${TEST_UNIT_COMMON_DIR}/mocks-rpma-utils.c
		${TEST_UNIT_COMMON_DIR}/mocks-rpma-srq.c
		${TEST_UNIT_COMMON_DIR}/mocks-rpma-srq_cfg.c
		${TEST_UNIT_COMMON_DIR}/mocks-stdlib.c
		${LIBRPMA_SOURCE_DIR}/rpma_err.c
		${LIBRPMA_SOURCE_DIR}/peer.c)

	target_compile_definitions(${name} PRIVATE TEST_MOCK_ALLOC)

	set_target_properties(${name}
		PROPERTIES
		LINK_FLAGS "-Wl,--wrap=_test_malloc")

	add_test_generic(NAME ${name} TRACERS none)
endfunction()

add_test_peer(create_qp)
add_test_peer(create_srq)
add_test_peer(mr_reg)
add_test_peer(new)
