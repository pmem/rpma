#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2020, Intel Corporation
#

cmake_minimum_required(VERSION 3.3)
project(messages-ping-pong C)

set(LIBIBVERBS_REQUIRED_VERSION 1.0) # XXX set the required value

find_package(PkgConfig QUIET)

if(PKG_CONFIG_FOUND)
	pkg_check_modules(LIBRPMA REQUIRED librpma)
	pkg_check_modules(LIBIBVERBS REQUIRED libibverbs>=${LIBIBVERBS_REQUIRED_VERSION})
else()
	find_package(LIBRPMA REQUIRED)
	find_package(LIBIBVERBS ${LIBIBVERBS_REQUIRED_VERSION} REQUIRED)
endif()

link_directories(${LIBRPMA_LIBRARY_DIRS} ${LIBIBVERBS_LIBRARY_DIRS})

function(add_example name)
	set(srcs ${ARGN})
	add_executable(example-${name} ${srcs})
	target_include_directories(example-${name}
		PRIVATE
			${LIBRPMA_INCLUDE_DIRS}
			${LIBIBVERBS_INCLUDE_DIRS}
			../common)
	target_link_libraries(example-${name} rpma ${LIBIBVERBS_LIBRARIES})
endfunction()

add_example(server server.c ../common/common-conn.c)
add_example(client client.c ../common/common-conn.c)
