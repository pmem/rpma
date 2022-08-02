#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2020-2022, Intel Corporation
# Copyright 2021-2022, Fujitsu
#

include(../../cmake/ctest_helpers.cmake)

function(add_test_conn_cfg name)
	set(src_name conn_cfg-${name})
	set(name ut-${src_name})
	build_test_src(UNIT NAME ${name} SRCS
		${src_name}.c
		conn_cfg-common.c
		${TEST_UNIT_COMMON_DIR}/mocks-stdlib.c
		${LIBRPMA_SOURCE_DIR}/conn_cfg.c)

	target_compile_definitions(${name} PRIVATE TEST_MOCK_ALLOC)

	set_target_properties(${name}
		PROPERTIES
		LINK_FLAGS "-Wl,--wrap=_test_malloc")

	add_test_generic(NAME ${name} TRACERS none)
endfunction()

add_test_conn_cfg(compl_channel)
add_test_conn_cfg(cqe)
add_test_conn_cfg(cq_size)
add_test_conn_cfg(delete)
add_test_conn_cfg(new)
add_test_conn_cfg(rcqe)
add_test_conn_cfg(rcq_size)
add_test_conn_cfg(rq_size)
add_test_conn_cfg(sq_size)
add_test_conn_cfg(srq)
add_test_conn_cfg(timeout)
