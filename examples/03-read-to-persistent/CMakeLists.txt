#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2020, Intel Corporation
# Copyright 2021, Fujitsu
#

cmake_minimum_required(VERSION 3.3)
project(read-to-persistent C)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}
	${CMAKE_SOURCE_DIR}/../cmake
	${CMAKE_SOURCE_DIR}/../../cmake)

include(${CMAKE_SOURCE_DIR}/../../cmake/functions.cmake)
# set LIBRT_LIBRARIES if linking with librt is required
check_if_librt_is_required()

set(LIBPMEM_REQUIRED_VERSION 1.6)

find_package(PkgConfig QUIET)

if(PKG_CONFIG_FOUND)
	pkg_check_modules(LIBRPMA librpma)
	pkg_check_modules(LIBIBVERBS libibverbs)
	pkg_check_modules(LIBPMEM QUIET libpmem>=${LIBPMEM_REQUIRED_VERSION})
endif()
if(NOT LIBRPMA_FOUND)
	find_package(LIBRPMA REQUIRED librpma)
endif()
if(NOT LIBIBVERBS_FOUND)
	find_package(LIBIBVERBS REQUIRED libibverbs)
endif()
if(NOT LIBPMEM_FOUND)
	find_package(LIBPMEM ${LIBPMEM_REQUIRED_VERSION} QUIET)
endif()

link_directories(${LIBRPMA_LIBRARY_DIRS})

function(add_example name)
	set(srcs ${ARGN})
	add_executable(${name} ${srcs})
	target_include_directories(${name}
		PRIVATE
			${LIBRPMA_INCLUDE_DIRS}
			../common)
	target_link_libraries(${name} rpma ${LIBIBVERBS_LIBRARIES} ${LIBRT_LIBRARIES})

	if(LIBPMEM_FOUND)
		target_include_directories(${name}
			PRIVATE ${LIBPMEM_INCLUDE_DIRS})
		target_link_libraries(${name} ${LIBPMEM_LIBRARIES})
		target_compile_definitions(${name}
			PRIVATE USE_LIBPMEM)
	endif()
endfunction()

add_example(server server.c ../common/common-conn.c)
add_example(client client.c ../common/common-conn.c)
