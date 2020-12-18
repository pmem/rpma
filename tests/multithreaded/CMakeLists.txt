#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2018-2020, Intel Corporation
#

add_custom_target(multithreaded)
include_directories(${LIBRPMA_INCLUDE_DIRS})
link_directories(${LIBRPMA_LIBRARY_DIRS})

function(add_multithreaded)
	set(options USE_LIBIBVERBS)
	set(oneValueArgs NAME BIN)
	set(multiValueArgs SRCS)
	cmake_parse_arguments(MULTITHREADED
		"${options}"
		"${oneValueArgs}"
		"${multiValueArgs}"
		${ARGN})

	set(target multithreaded-${MULTITHREADED_NAME}-${MULTITHREADED_BIN})

	prepend(srcs ${CMAKE_CURRENT_SOURCE_DIR} ${srcs})
	add_executable(${target} ${MULTITHREADED_SRCS})
	add_dependencies(multithreaded ${target})
	set_target_properties(${target} PROPERTIES
		OUTPUT_NAME ${MULTITHREADED_BIN}
		RUNTIME_OUTPUT_DIRECTORY ${MULTITHREADED_NAME})
	target_link_libraries(${target} ${LIBRPMA_LIBRARIES} pthread)

	if(MULTITHREADED_USE_LIBIBVERBS)
		target_include_directories(${target}
			PRIVATE ${LIBIBVERBS_INCLUDE_DIRS})
		target_link_libraries(${target} ${LIBIBVERBS_LIBRARIES})
	endif()
endfunction()

add_multithreaded(NAME utils BIN 01-rpma_utils_get_ibv_context
	SRCS utils/01-rpma_utils_get_ibv_context.c)

add_subdirectory(utils)
