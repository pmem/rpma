#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2020, Intel Corporation
#

include(../cmake/ctest_helpers.cmake)

build_test_src(utils-test
	utils-test.c
	${LIBRPMA_SOURCE_DIR}/rpma.c)

target_link_libraries(utils-test test-rpma-utils)

add_test_generic(NAME utils-test TRACERS none)
