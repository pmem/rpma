#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2020, Intel Corporation
#

include(../../cmake/ctest_helpers.cmake)

build_test_src(mr-test
	mr-test.c
	mr-test-common.c
	mr-test-descriptor.c
	mr-test-read.c
	mr-test-reg.c
	mr-test-write.c
	${LIBRPMA_SOURCE_DIR}/mr.c)

target_link_libraries(mr-test test-rpma-utils)

set_target_properties(mr-test
	PROPERTIES
	LINK_FLAGS "-Wl,--wrap=_test_malloc")

add_test_generic(NAME mr-test TRACERS none)
