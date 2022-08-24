#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2018-2022, Intel Corporation
#

include(cmake/ctest_helpers.cmake)

add_custom_target(tests)

add_flag(-Wall)

find_packages()

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
			"There are possible two alternative actions:\n"
			"a) Run 'make config_softroce' to configure SoftRoCE"
			"   and to get the IP of a SoftRoCE-configured network interface"
			"   and set the RPMA_TESTING_IP environment variable to silent this message.\n"
			"or:\n"
			"b) Set the TESTS_RDMA_CONNECTION cmake variable to OFF in order to disable tests"
			"   that require a configured RDMA-capable network interface.")
	elseif("$ENV{RPMA_TESTING_IP}" STREQUAL "127.0.0.1")
		message(FATAL_ERROR "RPMA_TESTING_IP cannot be set to '127.0.0.1' "
			"(use an IP address of a configured RDMA-capable network interface instead)")
	else()
		message(STATUS "IP address used as a configured RDMA-capable network interface "
			"(defined by the RPMA_TESTING_IP environment variable): $ENV{RPMA_TESTING_IP}")
	endif()
else()
	message(STATUS "NOTICE: all multi-threaded and integration tests will be disabled "
			"because TESTS_RDMA_CONNECTION is OFF"
			"(they require a configured RDMA-capable network interface)")
endif()

# set NPROC to number of processing units available
execute_process(COMMAND nproc
		OUTPUT_VARIABLE NPROC
		WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
		OUTPUT_STRIP_TRAILING_WHITESPACE
		ERROR_QUIET)

message(STATUS "Number of processing units available: ${NPROC}")

math(EXPR NPROCdiv2 "${NPROC} / 2")
# We assume 16 threads is maximum for drd and helgrind
if(NPROCdiv2 LESS 16)
	set(MAX_THREADS ${NPROCdiv2})
else()
	set(MAX_THREADS 16)
endif()
message(STATUS "Maximum number of threads for drd and helgrind: ${MAX_THREADS}")

add_subdirectory(unit)
if(TESTS_RDMA_CONNECTION AND VALGRIND_FOUND)
	add_subdirectory(multithreaded)
endif()
