#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2022, Intel Corporation
#

include(../../cmake/ctest_helpers.cmake)

add_multithreaded(NAME conn_cfg BIN get_cq_size
	SRCS rpma_conn_cfg_get_cq_size.c rpma_conn_cfg_common.c rpma_conn_cfg_common_get.c)
add_multithreaded(NAME conn_cfg BIN get_compl_channel
	SRCS rpma_conn_cfg_get_compl_channel.c rpma_conn_cfg_common.c rpma_conn_cfg_common_get.c)
add_multithreaded(NAME conn_cfg BIN get_rq_size
	SRCS rpma_conn_cfg_get_rq_size.c rpma_conn_cfg_common.c rpma_conn_cfg_common_get.c)
add_multithreaded(NAME conn_cfg BIN get_sq_size
	SRCS rpma_conn_cfg_get_sq_size.c rpma_conn_cfg_common.c rpma_conn_cfg_common_get.c)
add_multithreaded(NAME conn_cfg BIN get_srq
	SRCS rpma_conn_cfg_get_srq.c rpma_conn_cfg_common.c rpma_conn_cfg_common_get.c)
add_multithreaded(NAME conn_cfg BIN get_rcq_size
	SRCS rpma_conn_cfg_get_rcq_size.c rpma_conn_cfg_common.c rpma_conn_cfg_common_get.c)
add_multithreaded(NAME conn_cfg BIN get_timeout
	SRCS rpma_conn_cfg_get_timeout.c rpma_conn_cfg_common.c rpma_conn_cfg_common_get.c)
add_multithreaded(NAME conn_cfg BIN new
	SRCS rpma_conn_cfg_new.c)
add_multithreaded(NAME conn_cfg BIN set_compl_channel
	SRCS rpma_conn_cfg_set_compl_channel.c rpma_conn_cfg_common.c rpma_conn_cfg_common_set.c)
add_multithreaded(NAME conn_cfg BIN set_cq_size
	SRCS rpma_conn_cfg_set_cq_size.c rpma_conn_cfg_common.c rpma_conn_cfg_common_set.c)
add_multithreaded(NAME conn_cfg BIN set_rq_size
	SRCS rpma_conn_cfg_set_rq_size.c rpma_conn_cfg_common.c rpma_conn_cfg_common_set.c)
add_multithreaded(NAME conn_cfg BIN set_sq_size
	SRCS rpma_conn_cfg_set_sq_size.c rpma_conn_cfg_common.c rpma_conn_cfg_common_set.c)
add_multithreaded(NAME conn_cfg BIN set_srq
	SRCS rpma_conn_cfg_set_srq.c rpma_conn_cfg_common.c rpma_conn_cfg_common_set.c)
add_multithreaded(NAME conn_cfg BIN set_rcq_size
	SRCS rpma_conn_cfg_set_rcq_size.c rpma_conn_cfg_common.c rpma_conn_cfg_common_set.c)
add_multithreaded(NAME conn_cfg BIN set_timeout
	SRCS rpma_conn_cfg_set_timeout.c rpma_conn_cfg_common.c)
