#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2020-2021, Intel Corporation
# Copyright 2021, Fujitsu
#

include(../../cmake/ctest_helpers.cmake)

set(TARGET "example-01-connection")

build_test_src(NAME ${TARGET} SRCS
	example-01-connection.c
	../common/mocks.c
	${CMAKE_SOURCE_DIR}/examples/01-connection/client.c
	${CMAKE_SOURCE_DIR}/examples/01-connection/server.c
	${LIBRPMA_SOURCE_DIR}/conn.c
	${LIBRPMA_SOURCE_DIR}/conn_cfg.c
	${LIBRPMA_SOURCE_DIR}/conn_req.c
	${LIBRPMA_SOURCE_DIR}/cq.c
	${LIBRPMA_SOURCE_DIR}/flush.c
	${LIBRPMA_SOURCE_DIR}/ep.c
	${LIBRPMA_SOURCE_DIR}/info.c
	${LIBRPMA_SOURCE_DIR}/librpma.c
	${LIBRPMA_SOURCE_DIR}/log.c
	${LIBRPMA_SOURCE_DIR}/log_default.c
	${LIBRPMA_SOURCE_DIR}/mr.c
	${LIBRPMA_SOURCE_DIR}/peer.c
	${LIBRPMA_SOURCE_DIR}/peer_cfg.c
	${LIBRPMA_SOURCE_DIR}/private_data.c
	${LIBRPMA_SOURCE_DIR}/rpma.c
	${LIBRPMA_SOURCE_DIR}/rpma_err.c)

target_include_directories(${TARGET} PRIVATE
	../common
	${CMAKE_SOURCE_DIR}/examples/common)

set_target_properties(${TARGET}
       PROPERTIES
       LINK_FLAGS "-Wl,--wrap=_test_malloc,--wrap=mmap,--wrap=munmap,--wrap=fprintf")

target_compile_definitions(${TARGET}
       PUBLIC TEST_MOCK_MAIN
       PRIVATE TEST_MOCK_ALLOC
       PRIVATE SRCVERSION="0.0")

add_test_generic(NAME ${TARGET} TRACERS none)
