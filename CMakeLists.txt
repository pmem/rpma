#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2020, Intel Corporation
#

cmake_minimum_required(VERSION 3.3)

project(rpma C)

# disable in-source builds
if(${PROJECT_SOURCE_DIR} STREQUAL ${PROJECT_BINARY_DIR})
	message(FATAL_ERROR "In-source build is not allowed.")
endif()

set(VERSION_MAJOR 0)
set(VERSION_MINOR 0)
set(VERSION_PATCH 0)
set(VERSION_PRERELEASE wip)

set(VERSION ${VERSION_MAJOR}.${VERSION_MINOR})
if (VERSION_PATCH GREATER 0)
	set(VERSION ${VERSION}.${VERSION_PATCH})
endif()
if (VERSION_PRERELEASE)
	set(VERSION ${VERSION}-${VERSION_PRERELEASE})
endif()

set(LIBRPMA_LIBRARIES rpma)
set(LIBRPMA_LIBRARY_DIRS ${CMAKE_BINARY_DIR}/src/)
set(LIBRPMA_INCLUDE_DIRS
	${CMAKE_SOURCE_DIR}/src/include/
	${CMAKE_SOURCE_DIR}/src/common/)
set(LIBRPMA_SOURCE_DIR ${CMAKE_SOURCE_DIR}/src/)

set(LIBIBVERBS_REQUIRED_VERSION 1.0) # XXX set the required value
set(LIBRDMACM_REQUIRED_VERSION 1.0)  # XXX set the required value

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

option(TESTS_USE_FORCED_PMEM "run tests with PMEM_IS_PMEM_FORCE=1" OFF)
option(TESTS_USE_VALGRIND "enable tests with valgrind (if found)" ON)
option(TESTS_LONG "enable long running tests" OFF)
option(TESTS_USE_FAULT_INJECTION "run tests with fault injection" OFF)

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

find_package(PkgConfig QUIET)

if(NOT PKG_CONFIG_FOUND)
	message(FATAL_ERROR "Pkgconfig not found")
endif()

if(NOT PERL_FOUND)
	message(FATAL_ERROR "Perl not found")
endif()

pkg_check_modules(LIBIBVERBS REQUIRED libibverbs>=${LIBIBVERBS_REQUIRED_VERSION})
pkg_check_modules(LIBRDMACM REQUIRED librdmacm>=${LIBRDMACM_REQUIRED_VERSION})

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

add_custom_target(tests)

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

if(DEVELOPER_MODE)
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

install(DIRECTORY examples/ DESTINATION ${CMAKE_INSTALL_DOCDIR}/examples
	FILES_MATCHING PATTERN "*.[ch]")

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

pkg_check_modules(VALGRIND QUIET valgrind)

if(VALGRIND_FOUND)
	add_flag(-DVG_MEMCHECK_ENABLED=1)
	add_flag(-DVG_DRD_ENABLED=1)
	add_flag(-DVG_HELGRIND_ENABLED=1)

	include_directories(${VALGRIND_INCLUDE_DIRS})
	find_pmemcheck()

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

if(NOT "${CPACK_GENERATOR}" STREQUAL "")
	include(${CMAKE_SOURCE_DIR}/cmake/packages.cmake)
endif()
