#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2020-2021, Intel Corporation
#

include(../../cmake/ctest_helpers.cmake)

add_multithreaded(NAME peer BIN rpma_peer_cfg_new
	SRCS rpma_peer_cfg_new.c)
add_multithreaded(NAME peer BIN rpma_peer_cfg_from_descriptor
	SRCS rpma_peer_cfg_from_descriptor.c)
add_multithreaded(NAME peer BIN rpma_peer_new
	SRCS rpma_peer_new.c)
