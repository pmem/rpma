#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2020, Intel Corporation
#

cmake_minimum_required(VERSION 3.3)
project(multiple-connections C)

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
	target_include_directories(example-${name}
		PRIVATE
			${LIBRPMA_INCLUDE_DIRS}
			../common)
	target_link_libraries(example-${name} rpma)
endfunction()

add_example(server server.c ../common/common-conn.c ../common/common-epoll.c)
add_example(client client.c ../common/common-conn.c)
