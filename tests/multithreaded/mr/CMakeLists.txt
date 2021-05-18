#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021, Intel Corporation
#

include(../../cmake/ctest_helpers.cmake)

add_cstyle(mr-all ${CMAKE_CURRENT_SOURCE_DIR}/*.[ch])
add_check_whitespace(mr-all ${CMAKE_CURRENT_SOURCE_DIR}/*.[ch])

add_multithreaded(NAME mr BIN rpma_mr_get_descriptor
	SRCS rpma_mr_get_descriptor.c)
add_multithreaded(NAME mr BIN rpma_mr_get_ptr
	SRCS rpma_mr_get_ptr.c)
add_multithreaded(NAME mr BIN rpma_mr_reg
	SRCS rpma_mr_reg.c)
