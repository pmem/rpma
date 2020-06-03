#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2020, Intel Corporation
#

include(../cmake/ctest_helpers.cmake)

build_test_lib(utils-test utils-test.c)
add_test_generic(NAME utils-test CASE 0 TRACERS none)
