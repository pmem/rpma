#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2020, Intel Corporation
#

cmake_minimum_required(VERSION 3.3)
project(template-example CXX)

include(FindThreads)

if(NOT WIN32)
	find_package(PkgConfig QUIET)
endif()

if(PKG_CONFIG_FOUND)
	pkg_check_modules(LIBRPMA REQUIRED librpma)
else()
	find_package(LIBRPMA REQUIRED)
endif()

link_directories(${LIBRPMA_LIBRARY_DIRS})

add_executable(template-example template-example.cpp)
target_include_directories(template-example PUBLIC ${LIBRPMA_INCLUDE_DIRS} . ..)
target_link_libraries(template-example ${LIBRPMA_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})

