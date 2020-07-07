#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2020, Intel Corporation
#

include(../../cmake/ctest_helpers.cmake)

build_test_src(conn_req-test
       conn_req-test.c
       ${LIBRPMA_SOURCE_DIR}/conn_req.c)

target_link_libraries(conn_req-test test-rpma-utils)

set_target_properties(conn_req-test
       PROPERTIES
       LINK_FLAGS "-Wl,--wrap=_test_malloc")

add_test_generic(NAME conn_req-test TRACERS none)
