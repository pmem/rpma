#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2018-2020, Intel Corporation
#

include(cmake/ctest_helpers.cmake)

add_cstyle(tests)

add_check_whitespace(tests)

add_check_whitespace(tests-cmake
	${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt)

add_cstyle(tests-all
       ${CMAKE_CURRENT_SOURCE_DIR}/*/*/*.c)
add_check_whitespace(tests-all
       ${CMAKE_CURRENT_SOURCE_DIR}/*/*/*.c)

add_library(test_backtrace STATIC test_backtrace.c)
if(LIBUNWIND_FOUND)
	target_compile_definitions(test_backtrace PUBLIC USE_LIBUNWIND=1)
endif()

add_library(test-rpma-utils STATIC
       ${LIBRPMA_SOURCE_DIR}/rpma_err.c
       ${LIBRPMA_SOURCE_DIR}/common/out.c
       ${LIBRPMA_SOURCE_DIR}/common/util.c)
target_include_directories(test-rpma-utils PRIVATE
       ${LIBRPMA_INCLUDE_DIRS}
       ${LIBRPMA_SOURCE_DIR})
target_compile_definitions(test-rpma-utils
       PUBLIC TEST_MOCK_ALLOC
       PRIVATE SRCVERSION="0.0")

add_custom_target(unit)
add_subdirectory(unit)
add_custom_target(integration)
add_subdirectory(integration)
