#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2020, Intel Corporation
#

include(../cmake/ctest_helpers.cmake)

build_test_lib(template-test template-test.c)
add_test_generic(NAME template-test CASE 0 TRACERS none)
