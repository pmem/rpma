#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2020, Intel Corporation
#

include(../cmake/ctest_helpers.cmake)

build_test_src(peer-test
       peer-test.c
       ${LIBRPMA_SOURCE_DIR}/peer.c
       ${LIBRPMA_SOURCE_DIR}/rpma_err.c
       ${LIBRPMA_SOURCE_DIR}/common/out.c
       ${LIBRPMA_SOURCE_DIR}/common/util.c)

target_compile_definitions(peer-test
       PRIVATE TEST_MOCK_ALLOC
       PRIVATE SRCVERSION="0.0")

set_target_properties(peer-test
       PROPERTIES
       LINK_FLAGS "-Wl,--wrap=_test_malloc")

add_test_generic(NAME peer-test CASE 0 TRACERS none)
