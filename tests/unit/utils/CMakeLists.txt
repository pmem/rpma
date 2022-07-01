#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2020-2022, Intel Corporation
#

include(../../cmake/ctest_helpers.cmake)

build_test_src(UNIT NAME ut-utils-conn_event_2str SRCS
	utils-conn_event_2str.c
	${TEST_UNIT_COMMON_DIR}/mocks-ibverbs.c
	${TEST_UNIT_COMMON_DIR}/mocks-rdma_cm.c
	${TEST_UNIT_COMMON_DIR}/mocks-rpma-info.c
	${TEST_UNIT_COMMON_DIR}/mocks-rpma-log.c
	${LIBRPMA_SOURCE_DIR}/rpma_err.c
	${LIBRPMA_SOURCE_DIR}/utils.c)

build_test_src(UNIT NAME ut-utils-get_ibv_context SRCS
	utils-get_ibv_context.c
	utils-common.c
	${TEST_UNIT_COMMON_DIR}/mocks-ibverbs.c
	${TEST_UNIT_COMMON_DIR}/mocks-rpma-log.c
	${LIBRPMA_SOURCE_DIR}/rpma_err.c
	${LIBRPMA_SOURCE_DIR}/utils.c)

add_test_generic(NAME ut-utils-conn_event_2str TRACERS none)
add_test_generic(NAME ut-utils-get_ibv_context TRACERS none)

if(ON_DEMAND_PAGING_SUPPORTED)
	build_test_src(UNIT NAME ut-utils-ibv_context_is_odp_capable SRCS
		utils-ibv_context_is_odp_capable.c
		${TEST_UNIT_COMMON_DIR}/mocks-ibverbs.c
		${TEST_UNIT_COMMON_DIR}/mocks-rdma_cm.c
		${TEST_UNIT_COMMON_DIR}/mocks-rpma-info.c
		${TEST_UNIT_COMMON_DIR}/mocks-rpma-log.c
		${LIBRPMA_SOURCE_DIR}/rpma_err.c
		${LIBRPMA_SOURCE_DIR}/utils.c)

	add_test_generic(NAME ut-utils-ibv_context_is_odp_capable TRACERS none)
endif()
