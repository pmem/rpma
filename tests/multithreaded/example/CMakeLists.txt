#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021, Intel Corporation
#

include(../../cmake/ctest_helpers.cmake)

add_cstyle(example-all ${CMAKE_CURRENT_SOURCE_DIR}/*.[ch])
add_check_whitespace(example-all ${CMAKE_CURRENT_SOURCE_DIR}/*.[ch])

add_multithreaded(NAME example BIN example SRCS example.c)
