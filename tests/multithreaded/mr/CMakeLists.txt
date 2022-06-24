#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021-2022, Intel Corporation
#

include(../../cmake/ctest_helpers.cmake)

add_multithreaded(NAME mr BIN rpma_mr_dereg
	SRCS rpma_mr_dereg.c)
add_multithreaded(NAME mr BIN rpma_mr_get_descriptor
	SRCS rpma_mr_get_descriptor.c)
add_multithreaded(NAME mr BIN rpma_mr_get_ptr
	SRCS rpma_mr_get_ptr.c)
add_multithreaded(NAME mr BIN rpma_mr_get_size
	SRCS rpma_mr_get_size.c)
add_multithreaded(NAME mr BIN rpma_mr_reg
	SRCS rpma_mr_reg.c)
add_multithreaded(NAME mr BIN rpma_mr_remote_from_descriptor
	SRCS rpma_mr_remote_from_descriptor.c server_rpma_mr_remote_from_descriptor.c
		../common/mtt_connect.c)
