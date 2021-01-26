#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2018-2021, Intel Corporation
#

include(../../cmake/ctest_helpers.cmake)

add_cstyle(utils-all ${CMAKE_CURRENT_SOURCE_DIR}/*.[ch])
add_check_whitespace(utils-all ${CMAKE_CURRENT_SOURCE_DIR}/*.[ch])

add_multithreaded(NAME utils BIN rpma_utils_get_ibv_context
	CASE 0
	SRCS rpma_utils_get_ibv_context.c)
add_multithreaded(NAME utils BIN rpma_utils_ibv_context_is_odp_capable
	CASE 0
	SRCS rpma_utils_ibv_context_is_odp_capable.c)
