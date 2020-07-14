#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2020, Intel Corporation
#

cmake_minimum_required(VERSION 3.3)
project(read-to-persistent C)

set(LIBPMEM2_REQUIRED_VERSION 1.9)

find_package(PkgConfig QUIET)

# Note: The exact version of libpmem2 is required only because its API is still
# experimental. The exact version should be replaced with any compatible (>=)
# when the libpmem2 API will have the first release.
if(PKG_CONFIG_FOUND)
	pkg_check_modules(LIBRPMA REQUIRED librpma)
	pkg_check_modules(LIBPMEM2 QUIET libpmem2=${LIBPMEM2_REQUIRED_VERSION})
else()
	find_package(LIBRPMA REQUIRED)
	find_package(LIBPMEM2 ${LIBPMEM2_REQUIRED_VERSION} EXACT QUIET)
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

	if(LIBPMEM2_FOUND)
		target_include_directories(example-${name}
			PRIVATE
				${LIBPMEM2_INCLUDE_DIRS})
		target_link_libraries(example-${name} libpmem2)
		target_compile_definitions(example-${name}
			PRIVATE USE_PMEM2)
	endif()
endfunction()

add_example(server server.c ../common/common.c)
add_example(client client.c ../common/common.c)
