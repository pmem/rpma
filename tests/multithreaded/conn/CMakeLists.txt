#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021-2022, Intel Corporation
#

include(../../cmake/ctest_helpers.cmake)

add_multithreaded(NAME conn BIN rpma_conn_apply_remote_peer_cfg
	SRCS rpma_conn_apply_remote_peer_cfg.c rpma_conn_common.c server_rpma_conn_common.c server_rpma_empty.c
		../common/mtt_connect.c ../common/mtt_common-epoll.c)

add_multithreaded(NAME conn BIN rpma_conn_get_private_data
	SRCS rpma_conn_get_private_data.c server_rpma_conn_get_private_data.c ../common/mtt_connect.c)

add_multithreaded(NAME conn BIN rpma_conn_req_connect
	SRCS rpma_conn_req_connect.c ../common/mtt_client_process.c ../common/mtt_connect.c)

add_multithreaded(NAME conn BIN rpma_conn_req_delete
	SRCS rpma_conn_req_delete.c)

add_multithreaded(NAME conn BIN rpma_conn_req_new
	SRCS rpma_conn_req_new.c)

add_multithreaded(NAME conn BIN rpma_read
	SRCS rpma_read.c rpma_conn_common.c server_rpma_conn_common.c server_rpma_empty.c
		../common/mtt_connect.c ../common/mtt_common-epoll.c)

add_multithreaded(NAME conn BIN rpma_send USE_LIBIBVERBS
	SRCS rpma_send.c rpma_conn_common.c server_rpma_conn_common.c server_rpma_send.c
		../common/mtt_connect.c ../common/mtt_common-epoll.c)

add_multithreaded(NAME conn BIN rpma_write
	SRCS rpma_write.c rpma_conn_common.c server_rpma_conn_common.c server_rpma_empty.c
		../common/mtt_connect.c ../common/mtt_common-epoll.c)

add_multithreaded(NAME conn BIN rpma_write_read
	SRCS rpma_write_read.c rpma_conn_common.c server_rpma_conn_common.c server_rpma_empty.c
		../common/mtt_connect.c ../common/mtt_common-epoll.c)
