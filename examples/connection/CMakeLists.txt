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
	add_executable(example-${name} ${srcs})
	target_include_directories(example-${name} PUBLIC ${LIBRPMA_INCLUDE_DIRS})
	target_link_libraries(example-${name} ${LIBRPMA_LIBRARIES})
endfunction()

add_example(connection-server connection-server.c)
add_example(connection-client connection-client.c)
