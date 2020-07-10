#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2020, Intel Corporation
#

include(../cmake/ctest_helpers.cmake)

build_test_src(private_data-test
       private_data-test.c
       ${LIBRPMA_SOURCE_DIR}/private_data.c)

target_compile_definitions(private_data-test PRIVATE TEST_MOCK_ALLOC)

set_target_properties(private_data-test
       PROPERTIES
       LINK_FLAGS "-Wl,--wrap=_test_malloc")

add_test_generic(NAME private_data-test TRACERS none)
