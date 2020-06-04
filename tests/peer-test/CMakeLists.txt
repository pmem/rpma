#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2020, Intel Corporation
#

include(../cmake/ctest_helpers.cmake)

build_test_src(peer-test
       peer-test.c
       ${LIBRPMA_SOURCE_DIR}/peer.c)

target_link_libraries(peer-test test-rpma-utils)

set_target_properties(peer-test
       PROPERTIES
       LINK_FLAGS "-Wl,--wrap=_test_malloc")

add_test_generic(NAME peer-test TRACERS none)
