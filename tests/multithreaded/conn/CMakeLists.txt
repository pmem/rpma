#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021, Intel Corporation
#

include(../../cmake/ctest_helpers.cmake)

add_cstyle(conn-all ${CMAKE_CURRENT_SOURCE_DIR}/*.[ch])
add_check_whitespace(conn-all ${CMAKE_CURRENT_SOURCE_DIR}/*.[ch])

add_multithreaded(NAME conn BIN rpma_conn_get_private_data
	SRCS rpma_conn_get_private_data.c server_rpma_conn_get_private_data.c)
