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

add_cstyle(tests-all
       ${CMAKE_CURRENT_SOURCE_DIR}/*/*.c)
add_check_whitespace(tests-all
       ${CMAKE_CURRENT_SOURCE_DIR}/*/*.c)

add_subdirectory(conn-test)
add_subdirectory(conn_req-test)
add_subdirectory(ep-test)
add_subdirectory(error-test)
add_subdirectory(info-test)
add_subdirectory(peer-test)
add_subdirectory(template-test)
add_subdirectory(utils-test)
add_subdirectory(log-test)
