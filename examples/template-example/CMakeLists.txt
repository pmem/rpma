#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2020, Intel Corporation
#

cmake_minimum_required(VERSION 3.3)
project(template-example C)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}
	${CMAKE_SOURCE_DIR}/../cmake
	${CMAKE_SOURCE_DIR}/../../cmake)

find_package(PkgConfig QUIET)

if(PKG_CONFIG_FOUND)
	pkg_check_modules(LIBRPMA librpma)
endif()
if(NOT LIBRPMA_FOUND)
	find_package(LIBRPMA REQUIRED librpma)
endif()

link_directories(${LIBRPMA_LIBRARY_DIRS})

add_executable(template-example template-example.c)
target_include_directories(template-example PUBLIC ${LIBRPMA_INCLUDE_DIRS} .)
target_link_libraries(template-example rpma ${CMAKE_THREAD_LIBS_INIT})

