#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021, Intel Corporation
#

include(../../cmake/ctest_helpers.cmake)

add_multithreaded(NAME ep BIN rpma_ep_get_fd
	SRCS rpma_ep_get_fd.c)
add_multithreaded(NAME ep BIN rpma_ep_listen
	SRCS rpma_ep_listen.c)
