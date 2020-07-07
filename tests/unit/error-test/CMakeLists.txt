#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2020, Intel Corporation
#

include(../../cmake/ctest_helpers.cmake)

build_test_src(error-test
	error-test.c
	${LIBRPMA_SOURCE_DIR}/rpma_err.c)

add_test_generic(NAME error-test TRACERS none)
