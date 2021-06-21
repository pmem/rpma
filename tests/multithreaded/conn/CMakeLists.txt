#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021, Intel Corporation
#

include(../../cmake/ctest_helpers.cmake)

add_multithreaded(NAME conn BIN rpma_conn_cfg_new
	SRCS rpma_conn_cfg_new.c)
add_multithreaded(NAME conn BIN rpma_conn_cfg_set_cq_size
	SRCS rpma_conn_cfg_set_cq_size.c)
add_multithreaded(NAME conn BIN rpma_conn_cfg_set_timeout
	SRCS rpma_conn_cfg_set_timeout.c)
add_multithreaded(NAME conn BIN rpma_conn_get_private_data
	SRCS rpma_conn_get_private_data.c server_rpma_conn_get_private_data.c)
add_multithreaded(NAME conn BIN rpma_conn_req_new
	SRCS rpma_conn_req_new.c)
