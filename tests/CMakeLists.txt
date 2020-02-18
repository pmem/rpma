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
add_flag(-Wpointer-arith)
add_flag(-Wunused-macros)
add_flag(-Wsign-conversion)
add_flag(-Wsign-compare)
add_flag(-Wunreachable-code-return)
add_flag(-Wmissing-variable-declarations)
add_flag(-fno-common)

add_flag(-ggdb DEBUG)
add_flag(-DDEBUG DEBUG)

add_flag("-U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=2" RELEASE)

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

build_test(template-test template-test/template-test.c)
add_test_generic(NAME template-test CASE 0 TRACERS none)

set(RPMA_CONFIG_SOURCES
	rpma_config/rpma_config.c
	../src/config.c
	../src/core/alloc.c
	../src/core/os_posix.c
	../src/core/os_thread_posix.c
	../src/core/out.c
	../src/core/util.c
	../src/core/util_posix.c)

build_test(rpma_config ${RPMA_CONFIG_SOURCES})
target_link_directories(rpma_config PRIVATE ${LIBRPMA_LIBRARY_DIRS})
target_compile_definitions(rpma_config PRIVATE SRCVERSION="${SRCVERSION}")
target_include_directories(rpma_config PRIVATE ${LIBRPMA_INCLUDE_DIRS} ../src/include ../src/core)
target_link_libraries(rpma_config rpma ${LIBRPMEM_LIBRARIES})
add_test_generic(NAME rpma_config CASE 0 TRACERS none)
