#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2020-2022, Intel Corporation
#

include(../../cmake/ctest_helpers.cmake)

function(add_test_peer_cfg name)
	set(src_name peer_cfg-${name})
	set(name ut-${src_name})
	build_test_src(UNIT NAME ${name} SRCS
		${src_name}.c
		peer_cfg-common.c
		${TEST_UNIT_COMMON_DIR}/mocks-rpma-log.c
		${TEST_UNIT_COMMON_DIR}/mocks-stdlib.c
		${LIBRPMA_SOURCE_DIR}/peer_cfg.c)

	target_compile_definitions(${name} PRIVATE TEST_MOCK_ALLOC)

	set_target_properties(${name}
		PROPERTIES
		LINK_FLAGS "-Wl,--wrap=_test_malloc")

	add_test_generic(NAME ${name} TRACERS none)
endfunction()

add_test_peer_cfg(delete)
add_test_peer_cfg(descriptor)
add_test_peer_cfg(direct_write_to_pmem)
add_test_peer_cfg(new)
