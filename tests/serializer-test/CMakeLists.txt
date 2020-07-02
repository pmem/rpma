#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2020, Intel Corporation
#

include(../cmake/ctest_helpers.cmake)

build_test_src(serializer-test
       serializer-test.c
       ${LIBRPMA_SOURCE_DIR}/serializer.c)

add_test_generic(NAME serializer-test TRACERS none)
