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

function(add_example example binary)
	set(srcs ${ARGN})
	set(target example-${example}-${binary})
	prepend(srcs ${CMAKE_CURRENT_SOURCE_DIR} ${srcs})
	add_executable(${target} ${srcs})
	set_target_properties(${target} PROPERTIES
		OUTPUT_NAME ${binary}
		RUNTIME_OUTPUT_DIRECTORY ${example})
	target_link_libraries(${target} ${LIBRPMA_LIBRARIES})
	target_include_directories(${target} PRIVATE common
	    ${LIBRPMA_SOURCE_DIR})
endfunction()

add_example(template template template-example/template-example.c)
add_example(01-connection server 01-connection/server.c)
add_example(01-connection client 01-connection/client.c)
add_example(02-read-to-volatile server
	02-read-to-volatile/server.c
	common/common.c)
add_example(02-read-to-volatile client
	02-read-to-volatile/client.c
	common/common.c)

add_example(log log log/log-example.c log/log-worker.c)

add_library(doc_snippets_template-snippet OBJECT doc_snippets/template-snippet.c)
