#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2020, Intel Corporation
#

cmake_minimum_required(VERSION 3.3)
project(connection-example C)

find_package(PkgConfig QUIET)

if(PKG_CONFIG_FOUND)
	pkg_check_modules(LIBRPMA REQUIRED librpma)
else()
	find_package(LIBRPMA REQUIRED)
endif()

link_directories(${LIBRPMA_LIBRARY_DIRS})

function(add_example name)
	set(srcs ${ARGN})
	add_executable(${name} ${srcs})
	target_include_directories(${name}
		PUBLIC ${LIBRPMA_INCLUDE_DIRS} ../common)
	target_link_libraries(${name} rpma)
endfunction()

add_example(server server.c ../common/common.c)
add_example(client client.c ../common/common.c)
