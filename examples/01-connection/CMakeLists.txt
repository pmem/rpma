#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2020-2021, Intel Corporation
#

cmake_minimum_required(VERSION 3.3)
project(connection-example C)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}
	${CMAKE_SOURCE_DIR}/../cmake
	${CMAKE_SOURCE_DIR}/../../cmake)

include(${CMAKE_SOURCE_DIR}/../../cmake/functions.cmake)
# set LIBRT_LIBRARIES if linking with librt is required
check_if_librt_is_required()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu99")

find_package(PkgConfig QUIET)

if(PKG_CONFIG_FOUND)
	pkg_check_modules(LIBRPMA librpma)
endif()
if(NOT LIBRPMA_FOUND)
	find_package(LIBRPMA REQUIRED librpma)
endif()

link_directories(${LIBRPMA_LIBRARY_DIRS})

function(add_example name)
	set(srcs ${ARGN})
	add_executable(${name} ${srcs})
	target_include_directories(${name} PUBLIC ${LIBRPMA_INCLUDE_DIRS})
	target_link_libraries(${name} rpma ${LIBRT_LIBRARIES})
endfunction()

add_example(server server.c)
add_example(client client.c)
