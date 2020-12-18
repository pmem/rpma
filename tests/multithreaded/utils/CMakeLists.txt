#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2018-2020, Intel Corporation
#

include(../../cmake/ctest_helpers.cmake)

add_cstyle(utils-all ${CMAKE_CURRENT_SOURCE_DIR}/*.[ch])
add_check_whitespace(utils-all ${CMAKE_CURRENT_SOURCE_DIR}/*.[ch])

add_multithreaded(NAME utils BIN rpma_utils_get_ibv_context
	SRCS rpma_utils_get_ibv_context.c)
