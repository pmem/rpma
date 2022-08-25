#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2022, Fujitsu
#

include(../../cmake/ctest_helpers.cmake)

function(add_test_srq_cfg name)
	set(src_name srq_cfg-${name})
	set(name ut-${src_name})
	build_test_src(UNIT NAME ${name} SRCS
		${src_name}.c
		srq_cfg-common.c
		${TEST_UNIT_COMMON_DIR}/mocks-stdlib.c
		${LIBRPMA_SOURCE_DIR}/srq_cfg.c)

	target_compile_definitions(${name} PRIVATE TEST_MOCK_ALLOC)

	set_target_properties(${name}
		PROPERTIES
		LINK_FLAGS "-Wl,--wrap=_test_malloc")

	add_test_generic(NAME ${name} TRACERS none)
endfunction()

add_test_srq_cfg(delete)
add_test_srq_cfg(new)
add_test_srq_cfg(rcqe)
add_test_srq_cfg(rcq_size)
add_test_srq_cfg(rq_size)
