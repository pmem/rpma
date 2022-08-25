#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2020-2022, Intel Corporation
#

include(../../cmake/ctest_helpers.cmake)

build_test_src(UNIT NAME ut-error SRCS
	error.c
	${LIBRPMA_SOURCE_DIR}/rpma_err.c)

add_test_generic(NAME ut-error TRACERS none)
