#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2020-2022, Intel Corporation
#

include(../../cmake/ctest_helpers.cmake)

build_test_lib(template template.c)
add_test_generic(NAME ut-template TRACERS none)
