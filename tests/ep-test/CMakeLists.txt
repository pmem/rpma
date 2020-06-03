#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2020, Intel Corporation
#

include(../cmake/ctest_helpers.cmake)

build_test_src(ep-test
       ep-test.c
       ${LIBRPMA_SOURCE_DIR}/ep.c)

target_link_libraries(ep-test test-rpma-utils)

set_target_properties(ep-test
       PROPERTIES
       LINK_FLAGS "-Wl,--wrap=_test_malloc")

add_test_generic(NAME ep-test CASE 0 TRACERS none)
