#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2018-2020, Intel Corporation
#

include(cmake/ctest_helpers.cmake)

add_cstyle(tests)

add_check_whitespace(tests)

add_check_whitespace(tests-cmake
	${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt)

add_flag(-Wall)

if(USE_ASAN)
       add_sanitizer_flag(address)
endif()
if(USE_UBSAN)
       add_sanitizer_flag(undefined)
endif()

if(COVERAGE)
       set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -coverage")
endif()

find_packages()

add_library(test_backtrace STATIC test_backtrace.c)
if(LIBUNWIND_FOUND)
	target_compile_definitions(test_backtrace PUBLIC USE_LIBUNWIND=1)
endif()

add_cstyle(tests-all
       ${CMAKE_CURRENT_SOURCE_DIR}/*/*.c)
add_check_whitespace(tests-all
       ${CMAKE_CURRENT_SOURCE_DIR}/*/*.c)

build_test_lib(template-test template-test/template-test.c)
add_test_generic(NAME template-test CASE 0 TRACERS none)

build_test_src(peer-test
       peer-test/peer-test.c
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

build_test_lib(utils-test utils-test/utils-test.c)
add_test_generic(NAME utils-test CASE 0 TRACERS none)
