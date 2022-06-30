#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2020-2022, Intel Corporation
#

cmake_minimum_required(VERSION 3.3)

function(add_example_with_pmem name)
	set(options USE_LIBPROTOBUFC)
	set(srcs ${ARGN})
	set(srcs ${srcs} ../common/common-hello.c)

	cmake_parse_arguments(EXAMPLE
		"${options}"
		${ARGN})

	if (EXAMPLE_USE_LIBPROTOBUFC AND NOT LIBPROTOBUFC_FOUND)
		message(STATUS "${target} skipped - no libprotobuf-c found")
		return()
	endif()

	if (LIBPMEM2_FOUND)
		set(srcs ${srcs} ../common/common-pmem2_map_file.c ../common/common-map_file_with_signature_check.c)
	elseif (LIBPMEM_FOUND)
		set(srcs ${srcs} ../common/common-pmem_map_file.c ../common/common-map_file_with_signature_check.c)
	endif()
	add_executable(${name} ${srcs})
	target_include_directories(${name}
		PRIVATE
			${LIBRPMA_INCLUDE_DIRS}
			../common)
	target_link_libraries(${name} rpma ${LIBIBVERBS_LIBRARIES} ${LIBRT_LIBRARIES})

	if(LIBPMEM2_FOUND)
		target_include_directories(${name}
			PRIVATE ${LIBPMEM2_INCLUDE_DIRS})
		target_link_libraries(${name} ${LIBPMEM2_LIBRARIES})
		target_compile_definitions(${name}
			PRIVATE USE_LIBPMEM2)
	elseif(LIBPMEM_FOUND)
		target_include_directories(${name}
			PRIVATE ${LIBPMEM_INCLUDE_DIRS})
		target_link_libraries(${name} ${LIBPMEM_LIBRARIES})
		target_compile_definitions(${name}
			PRIVATE USE_LIBPMEM)
	endif()

	if(EXAMPLE_USE_LIBPROTOBUFC)
		target_include_directories(${name}
			PRIVATE ${LIBPROTOBUFC_INCLUDE_DIRS})
		target_link_libraries(${name} ${LIBPROTOBUFC_LIBRARIES})
	endif()
endfunction()
