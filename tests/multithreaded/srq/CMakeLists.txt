#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2022, Intel Corporation
#

include(../../cmake/ctest_helpers.cmake)

add_multithreaded(NAME srq BIN rpma_srq_delete
	SRCS rpma_srq_delete.c)

add_multithreaded(NAME srq BIN rpma_srq_get_rcq
	SRCS rpma_srq_get_rcq.c)

add_multithreaded(NAME srq BIN rpma_srq_new
	SRCS rpma_srq_new.c)
