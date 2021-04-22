#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2020-2021, Intel Corporation
#

include(../../cmake/ctest_helpers.cmake)

add_cstyle(ep-all ${CMAKE_CURRENT_SOURCE_DIR}/*.[ch])
add_check_whitespace(ep-all ${CMAKE_CURRENT_SOURCE_DIR}/*.[ch])

add_multithreaded(NAME ep BIN rpma_ep_listen
	SRCS rpma_ep_listen.c)
