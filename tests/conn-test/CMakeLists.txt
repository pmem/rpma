#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2020, Intel Corporation
#

include(../cmake/ctest_helpers.cmake)

build_test_src(conn-test
       conn-test.c
       ${LIBRPMA_SOURCE_DIR}/conn.c)

target_link_libraries(conn-test test-rpma-utils)

set_target_properties(conn-test
       PROPERTIES
       LINK_FLAGS "-Wl,--wrap=_test_malloc")

add_test_generic(NAME conn-test TRACERS none)
