#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2020-2022, Intel Corporation
#

cmake_minimum_required(VERSION 3.3)

function(add_example_with_pmem)
	set(options USE_LIBPROTOBUFC)
	set(oneValueArgs NAME)
	set(multiValueArgs SRCS)
	cmake_parse_arguments(EXAMPLE
		"${options}"
		"${oneValueArgs}"
		"${multiValueArgs}"
		${ARGN})

	set(target ${EXAMPLE_NAME})
	
	if (EXAMPLE_USE_LIBPROTOBUFC AND NOT LIBPROTOBUFC_FOUND)
		message(STATUS "${target} skipped - no libprotobuf-c found")
		return()
	endif()

	prepend(srcs ${CMAKE_CURRENT_SOURCE_DIR} ${srcs})

	set(EXAMPLE_SRCS ${EXAMPLE_SRCS} ../common/common-conn.c ../common/common-hello.c)
	if (LIBPMEM_FOUND OR LIBPMEM2_FOUND)
		set(EXAMPLE_SRCS ${EXAMPLE_SRCS}
			../common/common-map_file_with_signature_check.c)
	endif()

	if (LIBPMEM2_FOUND)
		set(EXAMPLE_SRCS ${EXAMPLE_SRCS} ../common/common-pmem2_map_file.c)
	elseif (LIBPMEM_FOUND)
		set(EXAMPLE_SRCS ${EXAMPLE_SRCS} ../common/common-pmem_map_file.c)
	endif()
	add_executable(${target} ${EXAMPLE_SRCS})
	target_include_directories(${target} PRIVATE ../common
		${LIBRPMA_INCLUDE_DIR} ${LIBIBVERBS_INCLUDE_DIRS})
	target_link_libraries(${target} rpma ${LIBIBVERBS_LIBRARIES} ${LIBRT_LIBRARIES})

	if(LIBPMEM2_FOUND)
		target_include_directories(${target}
			PRIVATE ${LIBPMEM2_INCLUDE_DIRS})
		target_link_libraries(${target} ${LIBPMEM2_LIBRARIES})
		target_compile_definitions(${target}
			PRIVATE USE_LIBPMEM2)
	elseif(LIBPMEM_FOUND)
		target_include_directories(${target}
			PRIVATE ${LIBPMEM_INCLUDE_DIRS})
		target_link_libraries(${target} ${LIBPMEM_LIBRARIES})
		target_compile_definitions(${target}
			PRIVATE USE_LIBPMEM)
	endif()

	if(EXAMPLE_USE_LIBPROTOBUFC)
		target_include_directories(${target}
			PRIVATE ${LIBPROTOBUFC_INCLUDE_DIRS})
		target_link_libraries(${target} ${LIBPROTOBUFC_LIBRARIES})
	endif()

	if(IBV_ADVISE_MR_FLAGS_SUPPORTED)
		target_compile_definitions(${target} PRIVATE IBV_ADVISE_MR_FLAGS_SUPPORTED=1)
	endif()
endfunction()
