#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2020, Intel Corporation
# Copyright 2021, Fujitsu
#

include(../../cmake/ctest_helpers.cmake)

function(add_test_conn name)
	set(name conn-${name})
	build_test_src(UNIT NAME ${name} SRCS
		${name}.c
		conn-common.c
		${TEST_UNIT_COMMON_DIR}/mocks-ibverbs.c
		${TEST_UNIT_COMMON_DIR}/mocks-rdma_cm.c
		${TEST_UNIT_COMMON_DIR}/mocks-rpma-cq.c
		${TEST_UNIT_COMMON_DIR}/mocks-rpma-peer_cfg.c
		${TEST_UNIT_COMMON_DIR}/mocks-rpma-flush.c
		${TEST_UNIT_COMMON_DIR}/mocks-rpma-log.c
		${TEST_UNIT_COMMON_DIR}/mocks-rpma-mr.c
		${TEST_UNIT_COMMON_DIR}/mocks-rpma-utils.c
		${TEST_UNIT_COMMON_DIR}/mocks-stdlib.c
		${LIBRPMA_SOURCE_DIR}/rpma_err.c
		${LIBRPMA_SOURCE_DIR}/conn.c)

	target_compile_definitions(${name} PRIVATE TEST_MOCK_ALLOC)

	set_target_properties(${name}
		PROPERTIES
		LINK_FLAGS "-Wl,--wrap=_test_malloc")

	add_test_generic(NAME ${name} TRACERS none)
endfunction()

add_test_conn(apply_remote_peer_cfg)
add_test_conn(disconnect)
add_test_conn(flush)
add_test_conn(get_cq_rcq)
add_test_conn(get_event_fd)
add_test_conn(get_qp_num)
add_test_conn(new)
add_test_conn(next_event)
add_test_conn(private_data)
add_test_conn(read)
add_test_conn(recv)
add_test_conn(send)
add_test_conn(send_with_imm)
add_test_conn(write)
add_test_conn(write_atomic)
add_test_conn(write_with_imm)
