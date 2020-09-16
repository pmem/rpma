#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2020, Intel Corporation
#

include(../../cmake/ctest_helpers.cmake)

build_test_src(UNIT NAME utils-conn_event_2str SRCS
	utils-conn_event_2str.c
	${TEST_UNIT_COMMON_DIR}/mocks-ibverbs.c
	${TEST_UNIT_COMMON_DIR}/mocks-rdma_cm.c
	${TEST_UNIT_COMMON_DIR}/mocks-rpma-info.c
	${TEST_UNIT_COMMON_DIR}/mocks-rpma-log.c
	${LIBRPMA_SOURCE_DIR}/rpma_err.c
	${LIBRPMA_SOURCE_DIR}/rpma.c)

build_test_src(UNIT NAME utils-get_ibv_context SRCS
	utils-get_ibv_context.c
	utils-common.c
	${TEST_UNIT_COMMON_DIR}/mocks-ibverbs.c
	${TEST_UNIT_COMMON_DIR}/mocks-rpma-log.c
	${LIBRPMA_SOURCE_DIR}/rpma_err.c
	${LIBRPMA_SOURCE_DIR}/rpma.c)

build_test_src(UNIT NAME utils-ibv_context_is_odp_capable SRCS
	utils-ibv_context_is_odp_capable.c
	${TEST_UNIT_COMMON_DIR}/mocks-ibverbs.c
	${TEST_UNIT_COMMON_DIR}/mocks-rdma_cm.c
	${TEST_UNIT_COMMON_DIR}/mocks-rpma-info.c
	${TEST_UNIT_COMMON_DIR}/mocks-rpma-log.c
	${LIBRPMA_SOURCE_DIR}/rpma_err.c
	${LIBRPMA_SOURCE_DIR}/rpma.c)

if(NOT RDMA_GETADDRINFO_NEW_SIGNATURE)
	target_compile_definitions(utils-conn_event_2str PRIVATE RDMA_GETADDRINFO_OLD_SIGNATURE=1)
	target_compile_definitions(utils-get_ibv_context PRIVATE RDMA_GETADDRINFO_OLD_SIGNATURE=1)
	target_compile_definitions(utils-ibv_context_is_odp_capable PRIVATE RDMA_GETADDRINFO_OLD_SIGNATURE=1)
endif()

add_test_generic(NAME utils-conn_event_2str TRACERS none)
add_test_generic(NAME utils-get_ibv_context TRACERS none)
add_test_generic(NAME utils-ibv_context_is_odp_capable TRACERS none)
