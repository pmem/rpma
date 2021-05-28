#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021, Intel Corporation
#

include(../../cmake/ctest_helpers.cmake)

add_multithreaded(NAME example BIN example SRCS example.c)
