#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2020, Intel Corporation
#

include(../cmake/ctest_helpers.cmake)

build_test_src(mr-test
	mr-test.c
	${LIBRPMA_SOURCE_DIR}/rpma_err.c
 	${LIBRPMA_SOURCE_DIR}/mr.c)

target_compile_definitions(mr-test PUBLIC TEST_MOCK_ALLOC)

set_target_properties(mr-test
       PROPERTIES
       LINK_FLAGS "-Wl,--wrap=_test_malloc")

add_test_generic(NAME mr-test TRACERS none)
