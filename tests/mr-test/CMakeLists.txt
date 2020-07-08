#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2020, Intel Corporation
#

include(../cmake/ctest_helpers.cmake)

function(add_mr_test test)
	set(name mr-test-${test})
	build_test_src(${name}
		${name}.c
		mr-test-common.c
		${LIBRPMA_SOURCE_DIR}/mr.c)

	target_link_libraries(${name} test-rpma-utils)

	set_target_properties(${name}
		PROPERTIES
		LINK_FLAGS "-Wl,--wrap=_test_malloc")

	add_test_generic(NAME ${name} TRACERS none)
endfunction()

add_mr_test(descriptor)
add_mr_test(read)
add_mr_test(reg)
