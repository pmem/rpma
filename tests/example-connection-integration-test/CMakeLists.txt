#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2020, Intel Corporation
#

include(../cmake/ctest_helpers.cmake)

build_test_src(example-connection-integration-test
	example-connection-integration-test.c
	${CMAKE_SOURCE_DIR}/examples/01-connection/client.c
	${LIBRPMA_SOURCE_DIR}/common/alloc.c
	${LIBRPMA_SOURCE_DIR}/common/out.c
	${LIBRPMA_SOURCE_DIR}/common/util.c
	${LIBRPMA_SOURCE_DIR}/conn.c
	${LIBRPMA_SOURCE_DIR}/conn_req.c
	${LIBRPMA_SOURCE_DIR}/ep.c
	${LIBRPMA_SOURCE_DIR}/info.c
	${LIBRPMA_SOURCE_DIR}/librpma.c
	${LIBRPMA_SOURCE_DIR}/mr.c
	${LIBRPMA_SOURCE_DIR}/peer.c
	${LIBRPMA_SOURCE_DIR}/private_data.c
	${LIBRPMA_SOURCE_DIR}/rpma.c
	${LIBRPMA_SOURCE_DIR}/rpma_err.c)

set_target_properties(example-connection-integration-test
       PROPERTIES
       LINK_FLAGS "-Wl,--wrap=fprintf"
       LINK_FLAGS "-Wl,--wrap=_test_malloc")

target_compile_definitions(example-connection-integration-test
       PUBLIC TEST_MOCK_MAIN
       PUBLIC TEST_MOCK_ALLOC
       PRIVATE SRCVERSION="0.0")

add_test_generic(NAME example-connection-integration-test TRACERS none)
