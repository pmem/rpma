#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2022, Intel Corporation
#

include(../../cmake/ctest_helpers.cmake)

add_multithreaded(NAME log BIN rpma_log_set_function
	SRCS rpma_log_set_function.c)
