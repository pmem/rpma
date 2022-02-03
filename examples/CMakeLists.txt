#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2018-2022, Intel Corporation
# Copyright 2021, Fujitsu
#

add_flag(-Wall)
add_flag(-Wpointer-arith)
add_flag(-Wsign-compare)
add_flag(-Wunreachable-code-return)
add_flag(-Wmissing-variable-declarations)
add_flag(-fno-common)
add_flag(-Wunused-macros)
add_flag(-Wsign-conversion)

add_flag(-ggdb DEBUG)
add_flag(-DDEBUG DEBUG)

add_flag("-U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=2" RELEASE)

include(${CMAKE_SOURCE_DIR}/cmake/functions.cmake)
# set LIBRT_LIBRARIES if linking with librt is required
check_if_librt_is_required()

add_custom_target(examples)
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

add_cstyle(examples-all ${rpma_src_files})
add_check_whitespace(examples-all ${rpma_src_files})

function(add_example)
	set(options USE_LIBPMEM_IF_FOUND USE_LIBIBVERBS USE_LIBPROTOBUFC)
	set(oneValueArgs NAME BIN)
	set(multiValueArgs SRCS)
	cmake_parse_arguments(EXAMPLE
		"${options}"
		"${oneValueArgs}"
		"${multiValueArgs}"
		${ARGN})

	set(target example-${EXAMPLE_NAME}-${EXAMPLE_BIN})

	if (EXAMPLE_USE_LIBPROTOBUFC AND NOT LIBPROTOBUFC_FOUND)
		message(STATUS "${target} skipped - no libprotobuf-c found")
		return()
	endif()

	prepend(srcs ${CMAKE_CURRENT_SOURCE_DIR} ${srcs})
	add_executable(${target} ${EXAMPLE_SRCS})
	add_dependencies(examples ${target})
	set_target_properties(${target} PROPERTIES
		OUTPUT_NAME ${EXAMPLE_BIN}
		RUNTIME_OUTPUT_DIRECTORY ${EXAMPLE_NAME})
	target_link_libraries(${target} ${LIBRPMA_LIBRARIES} ${LIBRT_LIBRARIES})
	target_include_directories(${target} PRIVATE common
		${LIBRPMA_SOURCE_DIR})

	if(EXAMPLE_USE_LIBPMEM_IF_FOUND AND LIBPMEM_FOUND)
		target_include_directories(${target}
			PRIVATE ${LIBPMEM_INCLUDE_DIRS})
		target_link_libraries(${target} ${LIBPMEM_LIBRARIES})
		target_compile_definitions(${target}
			PRIVATE USE_LIBPMEM)
	endif()

	if(EXAMPLE_USE_LIBPMEM2_IF_FOUND AND LIBPMEM2_FOUND)
		target_include_directories(${target}
			PRIVATE ${LIBPMEM2_INCLUDE_DIRS})
		target_link_libraries(${target} ${LIBPMEM2_LIBRARIES})
		target_compile_definitions(${target}
			PRIVATE USE_LIBPMEM2)
	endif()
	
	if(EXAMPLE_USE_LIBIBVERBS)
		target_include_directories(${target}
			PRIVATE ${LIBIBVERBS_INCLUDE_DIRS})
		target_link_libraries(${target} ${LIBIBVERBS_LIBRARIES})
	endif()
	
	if(EXAMPLE_USE_LIBPROTOBUFC)
		target_include_directories(${target}
			PRIVATE ${LIBPROTOBUFC_INCLUDE_DIRS})
		target_link_libraries(${target} ${LIBPROTOBUFC_LIBRARIES})
	endif()
endfunction()

add_example(NAME template BIN template
	SRCS template-example/template-example.c)
add_example(NAME 01-connection BIN server
	SRCS 01-connection/server.c)
add_example(NAME 01-connection BIN client
	SRCS 01-connection/client.c)
add_example(NAME 02-read-to-volatile BIN server
	SRCS 02-read-to-volatile/server.c common/common-conn.c)
add_example(NAME 02-read-to-volatile BIN client USE_LIBIBVERBS
	SRCS 02-read-to-volatile/client.c common/common-conn.c)
add_example(NAME 03-read-to-persistent BIN server USE_LIBIBVERBS USE_LIBPMEM_IF_FOUND
	SRCS 03-read-to-persistent/server.c common/common-conn.c)
add_example(NAME 03-read-to-persistent BIN client USE_LIBPMEM_IF_FOUND
	SRCS 03-read-to-persistent/client.c common/common-conn.c)
add_example(NAME 04-write-to-persistent BIN server USE_LIBPMEM_IF_FOUND
	SRCS 04-write-to-persistent/server.c common/common-conn.c)
add_example(NAME 04-write-to-persistent BIN client USE_LIBIBVERBS USE_LIBPMEM_IF_FOUND
	SRCS 04-write-to-persistent/client.c common/common-conn.c)
add_example(NAME 05-flush-to-persistent BIN server USE_LIBPMEM_IF_FOUND
	SRCS 05-flush-to-persistent/server.c common/common-conn.c)
add_example(NAME 05-flush-to-persistent BIN client USE_LIBIBVERBS USE_LIBPMEM_IF_FOUND
	SRCS 05-flush-to-persistent/client.c common/common-conn.c)
add_example(NAME 06-multiple-connections BIN server USE_LIBIBVERBS USE_LIBPMEM_IF_FOUND
	SRCS 06-multiple-connections/server.c common/common-conn.c common/common-epoll.c)
add_example(NAME 06-multiple-connections BIN client USE_LIBPMEM_IF_FOUND
	SRCS 06-multiple-connections/client.c common/common-conn.c)
add_example(NAME 07-atomic-write BIN server USE_LIBPMEM_IF_FOUND
	SRCS 07-atomic-write/server.c common/common-conn.c)
add_example(NAME 07-atomic-write BIN client USE_LIBIBVERBS USE_LIBPMEM_IF_FOUND
	SRCS 07-atomic-write/client.c common/common-conn.c)
add_example(NAME 08-messages-ping-pong BIN server USE_LIBIBVERBS
	SRCS 08-messages-ping-pong/server.c common/common-conn.c
	08-messages-ping-pong/messages-ping-pong-common.c)
add_example(NAME 08-messages-ping-pong BIN client USE_LIBIBVERBS
	SRCS 08-messages-ping-pong/client.c common/common-conn.c
	08-messages-ping-pong/messages-ping-pong-common.c)
add_example(NAME 09-flush-to-persistent-GPSPM BIN server USE_LIBIBVERBS USE_LIBPMEM_IF_FOUND USE_LIBPROTOBUFC
	SRCS 09-flush-to-persistent-GPSPM/server.c common/common-conn.c
	09-flush-to-persistent-GPSPM/GPSPM_flush.pb-c.c)
add_example(NAME 09-flush-to-persistent-GPSPM BIN client USE_LIBIBVERBS USE_LIBPMEM_IF_FOUND USE_LIBPROTOBUFC
	SRCS 09-flush-to-persistent-GPSPM/client.c common/common-conn.c
	09-flush-to-persistent-GPSPM/GPSPM_flush.pb-c.c)
add_example(NAME 10-send-with-imm BIN server USE_LIBIBVERBS
	SRCS 10-send-with-imm/server.c common/common-conn.c)
add_example(NAME 10-send-with-imm BIN client USE_LIBIBVERBS
	SRCS 10-send-with-imm/client.c common/common-conn.c)
add_example(NAME 11-write-with-imm BIN server USE_LIBIBVERBS
	SRCS 11-write-with-imm/server.c common/common-conn.c)
add_example(NAME 11-write-with-imm BIN client USE_LIBIBVERBS
	SRCS 11-write-with-imm/client.c common/common-conn.c)
add_example(NAME 12-receive-completion-queue BIN server USE_LIBIBVERBS
	SRCS 12-receive-completion-queue/server.c common/common-conn.c
	12-receive-completion-queue/receive-completion-queue-common.c)
add_example(NAME 12-receive-completion-queue BIN client USE_LIBIBVERBS
	SRCS 12-receive-completion-queue/client.c common/common-conn.c
	12-receive-completion-queue/receive-completion-queue-common.c)

add_example(NAME log BIN log SRCS
	log/log-example.c
	log/log-worker.c
	${LIBRPMA_SOURCE_DIR}/log.c
	${LIBRPMA_SOURCE_DIR}/log_default.c)

add_library(doc_snippets_template-snippet OBJECT doc_snippets/template-snippet.c)
