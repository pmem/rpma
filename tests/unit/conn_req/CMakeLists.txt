#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2020-2021, Intel Corporation
# Copyright 2021, Fujitsu
#

include(../../cmake/ctest_helpers.cmake)

function(add_test_conn_req name)
       set(name conn_req-${name})
       build_test_src(UNIT NAME ${name} SRCS
              ${name}.c
              conn_req-common.c
              ${TEST_UNIT_COMMON_DIR}/mocks-ibverbs.c
              ${TEST_UNIT_COMMON_DIR}/mocks-rdma_cm.c
              ${TEST_UNIT_COMMON_DIR}/mocks-rpma-conn.c
              ${TEST_UNIT_COMMON_DIR}/mocks-rpma-conn_cfg.c
              ${TEST_UNIT_COMMON_DIR}/mocks-rpma-cq.c
              ${TEST_UNIT_COMMON_DIR}/mocks-rpma-info.c
              ${TEST_UNIT_COMMON_DIR}/mocks-rpma-log.c
              ${TEST_UNIT_COMMON_DIR}/mocks-rpma-mr.c
              ${TEST_UNIT_COMMON_DIR}/mocks-rpma-peer.c
              ${TEST_UNIT_COMMON_DIR}/mocks-rpma-private_data.c
              ${TEST_UNIT_COMMON_DIR}/mocks-stdlib.c
              ${LIBRPMA_SOURCE_DIR}/conn_req.c
              ${LIBRPMA_SOURCE_DIR}/rpma_err.c)

       target_compile_definitions(${name} PRIVATE TEST_MOCK_ALLOC)

       set_target_properties(${name}
              PROPERTIES
              LINK_FLAGS "-Wl,--wrap=_test_malloc")

       add_test_generic(NAME ${name} TRACERS none)
endfunction()

add_test_conn_req(connect)
add_test_conn_req(delete)
add_test_conn_req(from_cm_event)
add_test_conn_req(new)
add_test_conn_req(private_data)
add_test_conn_req(recv)
