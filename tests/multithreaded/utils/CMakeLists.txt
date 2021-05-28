#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2020-2021, Intel Corporation
#

include(../../cmake/ctest_helpers.cmake)

add_multithreaded(NAME utils BIN rpma_utils_get_ibv_context
	SRCS rpma_utils_get_ibv_context.c)
add_multithreaded(NAME utils BIN rpma_utils_ibv_context_is_odp_capable
	SRCS rpma_utils_ibv_context_is_odp_capable.c)
