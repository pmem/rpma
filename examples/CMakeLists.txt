#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2018-2020, Intel Corporation
#

if(MSVC_VERSION)
	add_flag(-W4)
else()
	add_flag(-Wall)
endif()
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

include_directories(
	${LIBRPMA_INCLUDE_DIRS}
	${CMAKE_CURRENT_SOURCE_DIR}/../src
	${CMAKE_CURRENT_SOURCE_DIR}/../src/include)

link_directories(${LIBRPMA_LIBRARY_DIRS})

add_cppstyle(examples-template-example
	${CMAKE_CURRENT_SOURCE_DIR}/template-example/*.[ch])
add_check_whitespace(examples-template-example
	${CMAKE_CURRENT_SOURCE_DIR}/template-example/*.[ch])

add_cppstyle(examples-doc_snippets
	${CMAKE_CURRENT_SOURCE_DIR}/doc_snippets/*.[ch])
add_check_whitespace(examples-doc_snippets
	${CMAKE_CURRENT_SOURCE_DIR}/doc_snippets/*.[ch])

function(add_example name)
	set(srcs ${ARGN})
	prepend(srcs ${CMAKE_CURRENT_SOURCE_DIR} ${srcs})
	add_executable(example-${name} ${srcs})
	target_include_directories(example-${name} PUBLIC ${LIBRPMA_INCLUDE_DIRS})
	target_link_libraries(example-${name} rpma ${LIBRPMA_LIBRARIES} ${LIBPMEM_LIBRARIES})
endfunction()

if(PKG_CONFIG_FOUND)
	pkg_check_modules(CURSES QUIET ncurses)
else()
	# Specifies that we want FindCurses to find ncurses and not just any
	# curses library
	set(CURSES_NEED_NCURSES TRUE)
	find_package(Curses QUIET)
endif()

add_example(template-example template-example/template-example.c)

add_library(doc_snippets_template-snippet OBJECT doc_snippets/template-snippet.c)
