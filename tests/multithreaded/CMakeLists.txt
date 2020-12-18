#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2018-2020, Intel Corporation
#

cmake_minimum_required(VERSION 3.3)
project(multithreaded-test C)

add_flag(-Wall)
add_flag(-Wpointer-arith)
add_flag(-Wsign-compare)
add_flag(-Wunreachable-code-return)
add_flag(-Wmissing-variable-declarations)
add_flag(-fno-common)
#add_flag(-Wunused-macros)
#add_flag(-Wsign-conversion)

add_flag(-ggdb DEBUG)
add_flag(-DDEBUG DEBUG)

add_flag(-lpthread)

add_flag("-U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=2" RELEASE)

include(${CMAKE_SOURCE_DIR}/cmake/functions.cmake)
# set LIBRT_LIBRARIES if linking with librt is required
check_if_librt_is_required()

add_custom_target(multithreaded)
include_directories(${LIBRPMA_INCLUDE_DIRS})
link_directories(${LIBRPMA_LIBRARY_DIRS})

file(GLOB src_files ${CMAKE_CURRENT_SOURCE_DIR}/*/*.[ch])

# Filter out protobuf-c generated files.
# Starting from CMake v3.6 we could use:
#    list(FILTER src_files EXCLUDE REGEX ".*(pb-c).*")
# but we require CMake v3.3, so we have to do it
# in the following way:
foreach(file IN LISTS src_files)
	if(NOT file MATCHES ".*(pb-c).*")
		set(rpma_src_files "${file};${rpma_src_files}")
	endif()
endforeach()

add_cstyle(multithreaded-all ${CMAKE_CURRENT_SOURCE_DIR}/*.[ch])
add_check_whitespace(multithreaded-all ${CMAKE_CURRENT_SOURCE_DIR}/*.[ch])

function(add_multithreaded)
	set(options USE_LIBPMEM_IF_FOUND USE_LIBIBVERBS USE_LIBPROTOBUFC)
	set(oneValueArgs NAME BIN)
	set(multiValueArgs SRCS)
	cmake_parse_arguments(MULTITHREADED
		"${options}"
		"${oneValueArgs}"
		"${multiValueArgs}"
		${ARGN})

	set(target multithreaded-${MULTITHREADED_NAME}-${MULTITHREADED_BIN})

	if (MULTITHREADED_USE_LIBPROTOBUFC AND NOT LIBPROTOBUFC_FOUND)
		message(STATUS "${target} skipped - no libprotobuf-c found")
		return()
	endif()

	prepend(srcs ${CMAKE_CURRENT_SOURCE_DIR} ${srcs})
	add_executable(${target} ${MULTITHREADED_SRCS})
	add_dependencies(multithreaded ${target})
	set_target_properties(${target} PROPERTIES
		OUTPUT_NAME ${MULTITHREADED_BIN}
		RUNTIME_OUTPUT_DIRECTORY ${MULTITHREADED_NAME})
	target_link_libraries(${target} ${LIBRPMA_LIBRARIES} ${LIBRT_LIBRARIES})
	target_include_directories(${target} PRIVATE common
		${LIBRPMA_SOURCE_DIR})

	if(MULTITHREADED_USE_LIBPMEM_IF_FOUND AND LIBPMEM_FOUND)
		target_include_directories(${target}
			PRIVATE ${LIBPMEM_INCLUDE_DIRS})
		target_link_libraries(${target} ${LIBPMEM_LIBRARIES})
		target_compile_definitions(${target}
			PRIVATE USE_LIBPMEM)
	endif()

	if(MULTITHREADED_USE_LIBIBVERBS)
		target_include_directories(${target}
			PRIVATE ${LIBIBVERBS_INCLUDE_DIRS})
		target_link_libraries(${target} ${LIBIBVERBS_LIBRARIES})
	endif()

	if(MULTITHREADED_USE_LIBPROTOBUFC)
		target_include_directories(${target}
			PRIVATE ${LIBPROTOBUFC_INCLUDE_DIRS})
		target_link_libraries(${target} ${LIBPROTOBUFC_LIBRARIES})
	endif()
endfunction()

add_multithreaded(NAME 01-rpma_utils_get_ibv_context BIN 01-rpma_utils_get_ibv_context
	SRCS 01-rpma_utils_get_ibv_context.c)
