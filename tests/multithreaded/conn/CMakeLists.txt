#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021, Intel Corporation
#

include(../../cmake/ctest_helpers.cmake)

add_cstyle(conn-all ${CMAKE_CURRENT_SOURCE_DIR}/*.[ch])
add_check_whitespace(conn-all ${CMAKE_CURRENT_SOURCE_DIR}/*.[ch])

add_multithreaded(NAME conn BIN rpma_conn_get_private_data
	# XXX Examples are used only as a proof of concept.
	# Please do not use examples for any other future tests.
	SRCS rpma_conn_get_private_data.c
		${CMAKE_SOURCE_DIR}/examples/02-read-to-volatile/server.c
		${CMAKE_SOURCE_DIR}/examples/common/common-conn.c
	USE_EXAMPLES)
