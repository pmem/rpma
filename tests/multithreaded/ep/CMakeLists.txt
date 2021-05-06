#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021, Intel Corporation
#

include(../../cmake/ctest_helpers.cmake)

add_cstyle(ep-all ${CMAKE_CURRENT_SOURCE_DIR}/*.[ch])
add_check_whitespace(ep-all ${CMAKE_CURRENT_SOURCE_DIR}/*.[ch])

add_multithreaded(NAME ep BIN rpma_ep_get_fd
	SRCS rpma_ep_get_fd.c)
add_multithreaded(NAME ep BIN rpma_ep_listen
	SRCS rpma_ep_listen.c)
add_multithreaded(NAME ep BIN rpma_ep_next_conn_req
	# XXX Examples are used only as a proof of concept.
	# Please do not use examples for any other future tests.
	SRCS rpma_ep_next_conn_req.c
		${CMAKE_SOURCE_DIR}/examples/01-connection/client.c
		${CMAKE_SOURCE_DIR}/examples/common/common-conn.c
	USE_EXAMPLES)
