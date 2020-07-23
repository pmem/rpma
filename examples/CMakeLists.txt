#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2018-2020, Intel Corporation
#

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

add_flag("-U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=2" RELEASE)

if(USE_ASAN)
	add_sanitizer_flag(address)
endif()
if(USE_UBSAN)
	add_sanitizer_flag(undefined)
endif()

if(COVERAGE)
       set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -coverage")
endif()

include_directories(${LIBRPMA_INCLUDE_DIRS})
link_directories(${LIBRPMA_LIBRARY_DIRS})

add_cstyle(examples-all
	${CMAKE_CURRENT_SOURCE_DIR}/*/*.[ch])
add_check_whitespace(examples-all
	${CMAKE_CURRENT_SOURCE_DIR}/*/*.[ch])

function(add_example)
	set(options USE_LIBPMEM_IF_FOUND)
	set(oneValueArgs NAME BIN)
	set(multiValueArgs SRCS)
	cmake_parse_arguments(EXAMPLE
		"${options}"
		"${oneValueArgs}"
		"${multiValueArgs}"
		${ARGN})

	set(target example-${EXAMPLE_NAME}-${EXAMPLE_BIN})
	prepend(srcs ${CMAKE_CURRENT_SOURCE_DIR} ${srcs})
	add_executable(${target} ${EXAMPLE_SRCS})
	set_target_properties(${target} PROPERTIES
		OUTPUT_NAME ${EXAMPLE_BIN}
		RUNTIME_OUTPUT_DIRECTORY ${EXAMPLE_NAME})
	target_link_libraries(${target} ${LIBRPMA_LIBRARIES})
	target_include_directories(${target} PRIVATE common
		${LIBRPMA_SOURCE_DIR})

	if(EXAMPLE_USE_LIBPMEM_IF_FOUND AND LIBPMEM_FOUND)
		target_include_directories(${target}
			PRIVATE ${LIBPMEM_INCLUDE_DIRS})
		target_link_libraries(${target} ${LIBPMEM_LIBRARIES})
		target_compile_definitions(${target}
			PRIVATE USE_LIBPMEM)
	endif()
endfunction()

add_example(NAME template BIN template
	SRCS template-example/template-example.c)
add_example(NAME 01-connection BIN server
	SRCS 01-connection/server.c)
add_example(NAME 01-connection BIN client
	SRCS 01-connection/client.c)
add_example(NAME 02-read-to-volatile BIN server
	SRCS 02-read-to-volatile/server.c common/common.c)
add_example(NAME 02-read-to-volatile BIN client
	SRCS 02-read-to-volatile/client.c common/common.c)
add_example(NAME 03-read-to-persistent BIN server USE_LIBPMEM_IF_FOUND
	SRCS 03-read-to-persistent/server.c common/common.c)
add_example(NAME 03-read-to-persistent BIN client USE_LIBPMEM_IF_FOUND
	SRCS 03-read-to-persistent/client.c common/common.c)

add_example(NAME log BIN log SRCS log/log-example.c log/log-worker.c)

add_library(doc_snippets_template-snippet OBJECT doc_snippets/template-snippet.c)
