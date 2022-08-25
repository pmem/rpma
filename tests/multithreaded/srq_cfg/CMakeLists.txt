#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2022, Intel Corporation
#

include(../../cmake/ctest_helpers.cmake)

add_multithreaded(NAME srq_cfg BIN get_rcq_size
	SRCS rpma_srq_cfg_get_rcq_size.c rpma_srq_cfg_common.c)
add_multithreaded(NAME srq_cfg BIN get_rq_size
	SRCS rpma_srq_cfg_get_rq_size.c rpma_srq_cfg_common.c)
add_multithreaded(NAME srq_cfg BIN new
	SRCS rpma_srq_cfg_new.c)
add_multithreaded(NAME srq_cfg BIN set_rcq_size
	SRCS rpma_srq_cfg_set_rcq_size.c rpma_srq_cfg_common.c)
add_multithreaded(NAME srq_cfg BIN set_rq_size
	SRCS rpma_srq_cfg_set_rq_size.c rpma_srq_cfg_common.c)
