#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2020, Intel Corporation
#

include(../../cmake/ctest_helpers.cmake)

build_test_lib(template template.c)
add_test_generic(NAME template TRACERS none)
