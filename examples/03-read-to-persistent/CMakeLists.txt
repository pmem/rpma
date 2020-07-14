#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2020, Intel Corporation
#

cmake_minimum_required(VERSION 3.3)
project(read-to-persistent C)

set(LIBPMEM_REQUIRED_VERSION 1.6)

find_package(PkgConfig QUIET)

if(PKG_CONFIG_FOUND)
	pkg_check_modules(LIBRPMA REQUIRED librpma)
	pkg_check_modules(LIBPMEM QUIET libpmem>=${LIBPMEM_REQUIRED_VERSION})
else()
	find_package(LIBRPMA REQUIRED)
	find_package(LIBPMEM ${LIBPMEM_REQUIRED_VERSION} QUIET)
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

	if(LIBPMEM_FOUND)
		target_include_directories(example-${name}
			PRIVATE ${LIBPMEM_INCLUDE_DIRS})
		target_link_libraries(example-${name} ${LIBPMEM_LIBRARIES})
		target_compile_definitions(example-${name}
			PRIVATE USE_LIBPMEM)
	endif()
endfunction()

add_example(server server.c ../common/common.c)
add_example(client client.c ../common/common.c)
