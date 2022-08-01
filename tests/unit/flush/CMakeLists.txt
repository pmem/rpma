#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2020-2022, Intel Corporation
#

include(../../cmake/ctest_helpers.cmake)

function(add_test_flush name)
	set(src_name flush-${name})
	set(name ut-${src_name})
	build_test_src(UNIT NAME ${name} SRCS
		${src_name}.c
		flush-common.c
		${LIBRPMA_SOURCE_DIR}/rpma_err.c
		${LIBRPMA_SOURCE_DIR}/flush.c
		${TEST_UNIT_COMMON_DIR}/mocks-ibverbs.c
		${TEST_UNIT_COMMON_DIR}/mocks-rpma-log.c
		${TEST_UNIT_COMMON_DIR}/mocks-rpma-mr.c
		${TEST_UNIT_COMMON_DIR}/mocks-stdlib.c
		${TEST_UNIT_COMMON_DIR}/mocks-unistd.c)

	target_compile_definitions(${name} PRIVATE TEST_MOCK_ALLOC)

	set_target_properties(${name}
		PROPERTIES
		LINK_FLAGS "-Wl,--wrap=_test_malloc,--wrap=mmap,--wrap=munmap,--wrap=sysconf")

	add_test_generic(NAME ${name} TRACERS none)
endfunction()

add_test_flush(apm_execute)
add_test_flush(new)
