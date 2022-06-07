#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2018-2022, Intel Corporation
#

include(cmake/ctest_helpers.cmake)

add_custom_target(tests)

add_flag(-Wall)

find_packages()

add_library(test_backtrace STATIC test_backtrace.c)
if(LIBUNWIND_FOUND)
	target_compile_definitions(test_backtrace PUBLIC USE_LIBUNWIND=1)
endif()

add_cstyle(tests-all
		${CMAKE_CURRENT_SOURCE_DIR}/*.[ch]
		${CMAKE_CURRENT_SOURCE_DIR}/*/*/*.[ch])
add_check_whitespace(tests-all
		${CMAKE_CURRENT_SOURCE_DIR}/*.[ch]
		${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt
		${CMAKE_CURRENT_SOURCE_DIR}/*/CMakeLists.txt
		${CMAKE_CURRENT_SOURCE_DIR}/*/*/*.[ch]
		${CMAKE_CURRENT_SOURCE_DIR}/*/*/CMakeLists.txt)

if(TESTS_RDMA_CONNECTION)
	if("$ENV{RPMA_TESTING_IP}" STREQUAL "")
		message(WARNING "\nWARNING: The RPMA_TESTING_IP environment variable is not set! "
			"It should contain an IP address of a configured RDMA-capable network interface.\n"
			"127.0.0.1 will be used instead, the tests requiring a configured RDMA-capable network interface can fail!\n"
			"There are possible two alternative actions:\n"
			"a) Run 'make config_softroce' to configure SoftRoCE"
			"   and to get the IP of a SoftRoCE-configured network interface"
			"   and set the RPMA_TESTING_IP environment variable to silent this message.\n"
			"or:\n"
			"b) Set the TESTS_RDMA_CONNECTION cmake variable to OFF in order to disable tests"
			"   that require a configured RDMA-capable network interface.")
	else()
		message(STATUS "IP address used as a configured RDMA-capable network interface "
			"(defined by the RPMA_TESTING_IP environment variable): $ENV{RPMA_TESTING_IP}")
	endif()
endif()

add_subdirectory(unit)
add_subdirectory(integration)
if(TESTS_RDMA_CONNECTION)
	add_subdirectory(multithreaded)
endif()
