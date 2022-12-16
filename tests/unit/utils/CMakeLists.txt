#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2020-2022, Intel Corporation
# Copyright (c) 2022 Fujitsu Limited
#

include(../../cmake/ctest_helpers.cmake)

function(add_test_utils name)
	set(src_name utils-${name})
	set(name ut-${src_name})
	build_test_src(UNIT NAME ${name} SRCS
		${src_name}.c
		${TEST_UNIT_COMMON_DIR}/mocks-ibverbs.c
		${TEST_UNIT_COMMON_DIR}/mocks-rdma_cm.c
		${TEST_UNIT_COMMON_DIR}/mocks-rpma-info.c
		${TEST_UNIT_COMMON_DIR}/mocks-rpma-log.c
		${LIBRPMA_SOURCE_DIR}/rpma_err.c
		${LIBRPMA_SOURCE_DIR}/utils.c)

	add_test_generic(NAME ${name} TRACERS none)
endfunction()

add_test_utils(conn_event_2str)

build_test_src(UNIT NAME ut-utils-get_ibv_context SRCS
	utils-get_ibv_context.c
	utils-common.c
	${TEST_UNIT_COMMON_DIR}/mocks-ibverbs.c
	${TEST_UNIT_COMMON_DIR}/mocks-rpma-log.c
	${LIBRPMA_SOURCE_DIR}/rpma_err.c
	${LIBRPMA_SOURCE_DIR}/utils.c)

add_test_generic(NAME ut-utils-get_ibv_context TRACERS none)

if(ON_DEMAND_PAGING_SUPPORTED)
	add_test_utils(ibv_context_is_odp_capable)
endif()

add_test_utils(ibv_context_is_atomic_write_capable)
