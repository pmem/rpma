#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2022, Intel Corporation
#

cmake_minimum_required(VERSION 3.3)
project(receive-completion-queue C)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}
	${CMAKE_SOURCE_DIR}/../cmake
	${CMAKE_SOURCE_DIR}/../../cmake)

include(${CMAKE_SOURCE_DIR}/../../cmake/functions.cmake)
# set LIBRT_LIBRARIES if linking with librt is required
check_if_librt_is_required()

find_package(PkgConfig QUIET)

if(PKG_CONFIG_FOUND)
	pkg_check_modules(LIBRPMA librpma)
	pkg_check_modules(LIBIBVERBS libibverbs)
endif()
if(NOT LIBRPMA_FOUND)
	find_package(LIBRPMA REQUIRED librpma)
endif()
if(NOT LIBIBVERBS_FOUND)
	find_package(LIBIBVERBS REQUIRED libibverbs)
endif()

link_directories(${LIBRPMA_LIBRARY_DIRS} ${LIBIBVERBS_LIBRARY_DIRS})

function(add_example name)
	set(srcs ${ARGN})
	add_executable(${name} ${srcs})
	target_include_directories(${name}
		PRIVATE
			${LIBRPMA_INCLUDE_DIRS}
			${LIBIBVERBS_INCLUDE_DIRS}
			../common)
	target_link_libraries(${name} rpma ${LIBIBVERBS_LIBRARIES} ${LIBRT_LIBRARIES})
endfunction()

add_example(server server.c ../common/common-conn.c receive-completion-queue-common.c)
add_example(client client.c ../common/common-conn.c receive-completion-queue-common.c)
