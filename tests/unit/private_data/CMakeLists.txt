#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2020-2021, Intel Corporation
#

include(../../cmake/ctest_helpers.cmake)

function (add_test_private_data name)
       set(name private_data-${name})
       build_test_src(UNIT NAME ${name} SRCS
              ${name}.c
              private_data-common.c
              ${TEST_UNIT_COMMON_DIR}/mocks-stdlib.c
              ${LIBRPMA_SOURCE_DIR}/private_data.c)

       target_compile_definitions(${name} PRIVATE TEST_MOCK_ALLOC)

       set_target_properties(${name}
              PROPERTIES
              LINK_FLAGS "-Wl,--wrap=_test_malloc")

       add_test_generic(NAME ${name} TRACERS none)
endfunction()

add_test_private_data(store)
