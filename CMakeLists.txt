#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2020-2022, Intel Corporation
#

cmake_minimum_required(VERSION 3.3)

project(rpma C)

set(VERSION_MAJOR 0)
set(VERSION_MINOR 14)
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
include(CheckCCompilerFlag)
include(GNUInstallDirs)
include(${CMAKE_SOURCE_DIR}/cmake/functions.cmake)

option(BUILD_EXAMPLES "build examples" ON)
option(BUILD_TESTS "build tests" ON)
option(BUILD_DOC "build documentation" ON)

option(COVERAGE "run coverage test" OFF)
option(DEVELOPER_MODE "enable developer checks" OFF)
option(CHECK_CSTYLE "check code style of C sources" OFF)
option(TRACE_TESTS "more verbose test outputs" OFF)
option(USE_ASAN "enable AddressSanitizer (debugging)" OFF)
option(USE_UBSAN "enable UndefinedBehaviorSanitizer (debugging)" OFF)

option(TESTS_SOFT_ROCE "enable tests that require a SoftRoCE-configured network interface" ON)
option(TESTS_NO_FORTIFY_SOURCE "enable tests that do not pass when -D_FORTIFY_SOURCE=2 flag set" OFF)
option(TESTS_USE_FORCED_PMEM "run tests with PMEM_IS_PMEM_FORCE=1" OFF)
option(TESTS_USE_VALGRIND "enable tests with valgrind (if found)" OFF)
option(TESTS_USE_VALGRIND_PMEMCHECK "enable tests with valgrind pmemcheck (if found)" OFF)
option(TESTS_LONG "enable long running tests" OFF)
option(TESTS_USE_FAULT_INJECTION "run tests with fault injection" OFF)
option(TEST_PYTHON_TOOLS "run tests for Python tools" ON)

# Do not treat include directories from the interfaces
# of consumed Imported Targets as SYSTEM by default.
set(CMAKE_NO_SYSTEM_FROM_IMPORTED 1)

set(TEST_DIR ${CMAKE_CURRENT_BINARY_DIR}/test
	CACHE STRING "working directory for tests")

if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE "Debug")
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
	if(NOT LIBPMEM_FOUND)
		message(WARNING "Since libpmem is missing, the libpmem-based examples will be unable to use PMem. They will be using DRAM instead.")
	endif()
endif()

pkg_check_modules(LIBPMEM2 libpmem2>=${LIBPMEM2_REQUIRED_VERSION})
if(NOT LIBPMEM2_FOUND)
	find_package(LIBPMEM2 ${LIBPMEM2_REQUIRED_VERSION})
	if(NOT LIBPMEM2_FOUND)
		message(WARNING "Since libpmem2 is missing, the libpmem2-based examples will be unable to use PMem. They will be using DRAM instead.")
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

# check if librdmacm has correct signature of rdma_getaddrinfo()
check_signature_rdma_getaddrinfo(RDMA_GETADDRINFO_NEW_SIGNATURE)
if(RDMA_GETADDRINFO_NEW_SIGNATURE)
	message(STATUS "Using a new signature of rdma_getaddrinfo()")
else()
	message(STATUS "Using an old signature of rdma_getaddrinfo()")
	add_flag(-DRDMA_GETADDRINFO_OLD_SIGNATURE=1)
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
add_flag(-std=gnu99)
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

if(DEVELOPER_MODE)
	set(TESTS_NO_FORTIFY_SOURCE ON)

	add_flag(-Wall)
	add_flag(-Werror)

	if(CHECK_CSTYLE)
		add_dependencies(checkers cstyle)
	endif()
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
	add_flag(-DVG_MEMCHECK_ENABLED=1)
	add_flag(-DVG_DRD_ENABLED=1)
	add_flag(-DVG_HELGRIND_ENABLED=1)

	include_directories(${VALGRIND_INCLUDE_DIRS})

	if(TESTS_USE_VALGRIND_PMEMCHECK)
		find_pmemcheck()
	endif()

	if(VALGRIND_PMEMCHECK_FOUND)
		add_flag(-DVG_PMEMCHECK_ENABLED=1)
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

if(TEST_PYTHON_TOOLS)
	find_package(PYLINT REQUIRED pylint)
	add_subdirectory(tools/perf)
endif()

if(NOT "${CPACK_GENERATOR}" STREQUAL "")
	include(${CMAKE_SOURCE_DIR}/cmake/packages.cmake)
endif()

add_custom_target(config_softroce
	COMMAND ${CMAKE_SOURCE_DIR}/tools/config_softroce.sh)

add_custom_target(run_all_examples
	COMMAND ${CMAKE_SOURCE_DIR}/examples/run-all-on-SoftRoCE.sh ${CMAKE_BINARY_DIR}/examples)

add_custom_target(run_all_examples_under_valgrind
	COMMAND ${CMAKE_SOURCE_DIR}/examples/run-all-on-SoftRoCE.sh ${CMAKE_BINARY_DIR}/examples --valgrind)
