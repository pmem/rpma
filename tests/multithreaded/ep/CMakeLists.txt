#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021-2022, Intel Corporation
#

include(../../cmake/ctest_helpers.cmake)

add_multithreaded(NAME ep BIN rpma_ep_get_fd
	SRCS rpma_ep_get_fd.c)

add_multithreaded(NAME ep BIN rpma_ep_listen
	SRCS rpma_ep_listen.c)

add_multithreaded(NAME ep BIN rpma_ep_next_conn_req
	SRCS rpma_ep_next_conn_req.c ../common/mtt_client_process.c ../common/mtt_connect.c)

add_multithreaded(NAME ep BIN rpma_ep_shutdown
	SRCS rpma_ep_shutdown.c)
