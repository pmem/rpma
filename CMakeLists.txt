#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2020-2022, Intel Corporation
#

cmake_minimum_required(VERSION 3.3)

project(rpma C)

set(VERSION_MAJOR 1)
set(VERSION_MINOR 0)
set(VERSION_PATCH 0)
# set(VERSION_PRERELEASE rc1)

set(VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH})
if (VERSION_PRERELEASE)
	set(VERSION ${VERSION}-${VERSION_PRERELEASE})
endif()

set(LIBRPMA_LIBRARIES rpma)
set(LIBRPMA_LIBRARY_DIRS ${CMAKE_BINARY_DIR}/src/)
set(LIBRPMA_INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/src/include/)
set(LIBRPMA_SOURCE_DIR ${CMAKE_SOURCE_DIR}/src/)

# required only for some examples
set(LIBPMEM_REQUIRED_VERSION 1.6)
set(LIBPMEM2_REQUIRED_VERSION 1.10)

set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)

include(FindPerl)
include(FindThreads)
include(CMakePackageConfigHelpers)
include(GNUInstallDirs)
include(${CMAKE_SOURCE_DIR}/cmake/functions.cmake)

option(BUILD_DOC "build documentation" ON)
option(BUILD_TESTS "build tests" ON)
option(BUILD_EXAMPLES "build examples" ON)
option(BUILD_DEVELOPER_MODE "enable developer checks" OFF)

option(TESTS_COVERAGE "run coverage test" OFF)
option(TESTS_NO_FORTIFY_SOURCE "enable tests that do not pass when -D_FORTIFY_SOURCE=2 flag set" OFF)
option(TESTS_PERF_TOOLS "run tests for the performance analysis tools" OFF)
option(TESTS_RDMA_CONNECTION "enable tests that require a configured RDMA-capable network interface (valgrind required)" OFF)
option(TESTS_USE_FORCED_PMEM "run tests with PMEM_IS_PMEM_FORCE=1" OFF)
option(TESTS_USE_VALGRIND_PMEMCHECK "enable tests with valgrind pmemcheck (if found)" OFF)
option(TESTS_VERBOSE_OUTPUT "verbose test outputs" OFF)

option(DEBUG_LOG_TRACE "enable logging function traces" OFF)
option(DEBUG_FAULT_INJECTION "enable fault injection" OFF)
option(DEBUG_USE_ASAN "enable AddressSanitizer (-fsanitize=address)" OFF)
option(DEBUG_USE_UBSAN "enable UndefinedBehaviorSanitizer (-fsanitize=undefined)" OFF)

# Do not treat include directories from the interfaces
# of consumed Imported Targets as SYSTEM by default.
set(CMAKE_NO_SYSTEM_FROM_IMPORTED 1)

set(TEST_DIR ${CMAKE_CURRENT_BINARY_DIR}/test
	CACHE STRING "working directory for tests")

set(buildTypes Release Debug RelWithDebInfo MinSizeRel)

if(NOT CMAKE_BUILD_TYPE)
	message(STATUS "No build type selected (CMAKE_BUILD_TYPE), defaulting to Release")
	set(CMAKE_BUILD_TYPE "Release" CACHE STRING
		"Choose the type of build, options are: Release Debug RelWithDebInfo MinSizeRel ..." FORCE)
	set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS ${buildTypes})
else()
	message(STATUS "CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")
	if(NOT CMAKE_BUILD_TYPE IN_LIST buildTypes)
		message(WARNING "Unusual build type was set, please make sure it's a proper one. "
			"Only following are supported by default: ${buildTypes}.")
	endif()
endif()

if(EXISTS "${CMAKE_SOURCE_DIR}/.git")
	execute_process(COMMAND git describe
			OUTPUT_VARIABLE SRCVERSION
			WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
			OUTPUT_STRIP_TRAILING_WHITESPACE
			ERROR_QUIET)
	if(NOT SRCVERSION)
		execute_process(COMMAND git log -1 --format=%h
				OUTPUT_VARIABLE SRCVERSION
				WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
				OUTPUT_STRIP_TRAILING_WHITESPACE)
	endif()
else()
	execute_process(COMMAND cat .version
			OUTPUT_VARIABLE SRCVERSION
			WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
			OUTPUT_STRIP_TRAILING_WHITESPACE)
endif()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake)

find_package(PkgConfig REQUIRED)

if(NOT PERL_FOUND)
	message(FATAL_ERROR "Perl not found")
endif()

pkg_check_modules(LIBIBVERBS libibverbs)
if(NOT LIBIBVERBS_FOUND)
	find_package(LIBIBVERBS REQUIRED libibverbs)
endif()

pkg_check_modules(LIBRDMACM librdmacm)
if(NOT LIBRDMACM_FOUND)
	find_package(LIBRDMACM REQUIRED librdmacm)
endif()

pkg_check_modules(LIBPMEM libpmem>=${LIBPMEM_REQUIRED_VERSION})
if(NOT LIBPMEM_FOUND)
	find_package(LIBPMEM ${LIBPMEM_REQUIRED_VERSION})
endif()

pkg_check_modules(LIBPMEM2 libpmem2>=${LIBPMEM2_REQUIRED_VERSION})
if(NOT LIBPMEM2_FOUND)
	find_package(LIBPMEM2 ${LIBPMEM2_REQUIRED_VERSION})
endif()

if(NOT LIBPMEM2_FOUND)
	if(NOT LIBPMEM_FOUND)
		message(WARNING "Since libpmem2 and libpmem are missing, the examples will be unable to use PMem. They will be using DRAM instead.")
	else()
		message(STATUS "libpmem will be used for examples that use PMem.")
	endif()
else()
	# XXX adjust message when all examples are adapted to libpmem2
	# message(STATUS "libpmem2 will be used for examples that use PMem.")
	if(NOT LIBPMEM_FOUND)
		message(WARNING "Since libpmem is missing, the examples that are adapted only to libpmem will be unable to use PMem. They will be using DRAM instead.")
		message(STATUS "libpmem2 will be used for examples that use PMem and are adapted to libpmem2.")
	else()
		message(STATUS "libpmem2 will be used for examples that use PMem and are adapted to libpmem2, otherwise libpmem will be used.")
	endif()
endif()

pkg_check_modules(LIBPROTOBUFC libprotobuf-c)

# check if libibverbs has ODP support
is_ODP_supported(ON_DEMAND_PAGING_SUPPORTED)
if(ON_DEMAND_PAGING_SUPPORTED)
	message(STATUS "On-Demand Paging (ODP) in libibverbs supported - Success")
	add_flag(-DON_DEMAND_PAGING_SUPPORTED=1)
else()
	message(WARNING "On-Demand Paging (ODP) is NOT supported and will be disabled (too old version of libibverbs)!")
endif()

# check if libibverbs has ibv_advise_mr() support
is_ibv_advise_mr_supported(IBV_ADVISE_MR_SUPPORTED)
if(IBV_ADVISE_MR_SUPPORTED)
	message(STATUS "ibv_advise_mr() supported in libibverbs - Success")
	add_flag(-DIBV_ADVISE_MR_SUPPORTED=1)
else()
	message(WARNING "ibv_advise_mr() is NOT supported and will be disabled (too old version of libibverbs)!")
endif()

# check if all required IBV_ADVISE_MR* flags are supported
are_ibv_advise_flags_supported(IBV_ADVISE_MR_FLAGS_SUPPORTED)

# check if librdmacm has correct signature of rdma_getaddrinfo()
check_signature_rdma_getaddrinfo(RDMA_GETADDRINFO_NEW_SIGNATURE)
if(RDMA_GETADDRINFO_NEW_SIGNATURE)
	message(STATUS "Using a new signature of rdma_getaddrinfo()")
else()
	message(STATUS "Using an old signature of rdma_getaddrinfo()")
	add_flag(-DRDMA_GETADDRINFO_OLD_SIGNATURE=1)
endif()

# check if atomic operations are supported
atomic_operations_supported(ATOMIC_OPERATIONS_SUPPORTED)
if(ATOMIC_OPERATIONS_SUPPORTED)
	message(STATUS "atomic operations are supported")
	add_flag(-DATOMIC_OPERATIONS_SUPPORTED=1)
else()
	message(WARNING "atomic operations are NOT supported (too old gcc/clang compiler). Some *_set*() functions will NOT be thread-safe!")
endif()

add_custom_target(checkers ALL)
add_custom_target(cstyle)
add_custom_target(check-whitespace)
add_custom_target(check-license
	COMMAND ${CMAKE_SOURCE_DIR}/utils/check_license/check-headers.sh
		${CMAKE_SOURCE_DIR}
		BSD-3-Clause)

add_custom_target(check-commits
	COMMAND ${CMAKE_SOURCE_DIR}/utils/check-commits.sh)

add_custom_target(check-whitespace-main
	COMMAND ${PERL_EXECUTABLE}
		${CMAKE_SOURCE_DIR}/utils/check_whitespace
		${CMAKE_SOURCE_DIR}/utils/check_license/*
		${CMAKE_SOURCE_DIR}/README.md)

add_dependencies(check-whitespace check-whitespace-main)

add_flag(-Wpointer-arith)
add_flag(-Wunused-macros)
add_flag(-Wsign-conversion)
add_flag(-Wsign-compare)
add_flag(-Wunreachable-code-return)
add_flag(-Wmissing-variable-declarations)
add_flag(-fno-common)
add_flag(-std=gnu11)
add_flag(-ggdb DEBUG)
add_flag(-DDEBUG DEBUG)
add_flag("-U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=2" RELEASE)

if(DEBUG_USE_ASAN)
	add_sanitizer_flag(address)
endif()

if(DEBUG_USE_UBSAN)
	add_sanitizer_flag(undefined)
endif()

if(TESTS_COVERAGE)
       set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -coverage")
endif()

if(BUILD_DEVELOPER_MODE)
	set(TESTS_NO_FORTIFY_SOURCE ON)

	add_flag(-Wall)
	add_flag(-Werror)

	add_dependencies(checkers cstyle)
	add_dependencies(checkers check-whitespace)
	add_dependencies(checkers check-license)
	add_dependencies(checkers check-commits)
endif()

add_check_whitespace(cmake-main ${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt)
add_check_whitespace(cmake-helpers ${CMAKE_CURRENT_SOURCE_DIR}/cmake/*.cmake)

configure_file(${CMAKE_SOURCE_DIR}/cmake/librpma.pc.in
	${CMAKE_CURRENT_BINARY_DIR}/librpma.pc @ONLY)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/librpma.pc
	DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)

configure_file(
	"${CMAKE_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in"
	"${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
	IMMEDIATE @ONLY)

add_custom_target(uninstall
	COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)

configure_package_config_file(${CMAKE_SOURCE_DIR}/cmake/librpma-config.cmake.in
	${CMAKE_CURRENT_BINARY_DIR}/librpma-config.cmake
	INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/librpma/cmake
	PATH_VARS CMAKE_INSTALL_LIBDIR CMAKE_INSTALL_INCLUDEDIR)

write_basic_package_version_file(librpma-config-version.cmake
				VERSION ${VERSION}
				COMPATIBILITY AnyNewerVersion)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/librpma-config.cmake ${CMAKE_CURRENT_BINARY_DIR}/librpma-config-version.cmake
	DESTINATION ${CMAKE_INSTALL_LIBDIR}/librpma/cmake)

pkg_check_modules(VALGRIND valgrind)

if(VALGRIND_FOUND)
	include_directories(${VALGRIND_INCLUDE_DIRS})

	# sets VALGRIND_S_OPTION to "-s" if valgrind supports it or "" otherwise
	valgrind_check_s_option()

	add_flag(-DVG_MEMCHECK_ENABLED=1)
	add_flag(-DVG_DRD_ENABLED=1)
	add_flag(-DVG_HELGRIND_ENABLED=1)

	if(TESTS_USE_VALGRIND_PMEMCHECK)
		find_pmemcheck()
	endif()
	if(VALGRIND_PMEMCHECK_FOUND)
		add_flag(-DVG_PMEMCHECK_ENABLED=1)
	endif()
else()
	if(TESTS_RDMA_CONNECTION)
		message(FATAL_ERROR "Valgrind not found! - tests requiring a configured RDMA-capable network interface"
				"(all multi-threaded and integration tests) require also valgrind to be installed."
				"Install valgrind or set TESTS_RDMA_CONNECTION to OFF.")
	else()
		message(STATUS "NOTICE: valgrind not found! - the tests requiring valgrind "
				"(all multi-threaded and integration tests) will not be run!")
	endif()
endif()

add_subdirectory(src)

if(BUILD_TESTS)
	if(TEST_DIR)
		enable_testing()
	else()
		message(WARNING "TEST_DIR is empty - 'make test' will not work")
	endif()

	add_subdirectory(tests)
endif()

if(BUILD_DOC)
	add_subdirectory(doc)
endif()

if(BUILD_EXAMPLES)
	add_subdirectory(examples)
endif()

if(TESTS_PERF_TOOLS)
	find_package(PYLINT REQUIRED pylint)
	add_subdirectory(tools/perf)
endif()

if(NOT "${CPACK_GENERATOR}" STREQUAL "")
	include(${CMAKE_SOURCE_DIR}/cmake/packages.cmake)
endif()

add_custom_target(config_softroce
	COMMAND ${CMAKE_SOURCE_DIR}/tools/config_softroce.sh)

add_custom_target(run_all_examples
	COMMAND ${CMAKE_SOURCE_DIR}/examples/run-all-examples.sh ${CMAKE_BINARY_DIR}/examples)

add_custom_target(run_all_examples_under_valgrind
	COMMAND ${CMAKE_SOURCE_DIR}/examples/run-all-examples.sh ${CMAKE_BINARY_DIR}/examples --valgrind)

add_custom_target(run_all_examples_with_fault_injection
	COMMAND ${CMAKE_SOURCE_DIR}/examples/run-all-examples.sh ${CMAKE_BINARY_DIR}/examples --integration-tests)
