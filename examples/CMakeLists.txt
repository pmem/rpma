#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2018-2022, Intel Corporation
# Copyright 2021-2022, Fujitsu
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
	set(options USE_LIBPROTOBUFC)
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

	set(EXAMPLE_SRCS ${EXAMPLE_SRCS} common/common-conn.c)
	if (LIBPMEM_FOUND OR LIBPMEM2_FOUND)
		set(EXAMPLE_SRCS ${EXAMPLE_SRCS}
			common/common-map_file_with_signature_check.c common/common-hello.c)
	endif()
	if (LIBPMEM2_FOUND)
		add_executable(${target} ${EXAMPLE_SRCS} common/common-pmem2_map_file.c)
	elseif (LIBPMEM_FOUND)
		add_executable(${target} ${EXAMPLE_SRCS} common/common-pmem_map_file.c)
	else()
		add_executable(${target} ${EXAMPLE_SRCS})
	endif()
	add_dependencies(examples ${target})
	set_target_properties(${target} PROPERTIES
		OUTPUT_NAME ${EXAMPLE_BIN}
		RUNTIME_OUTPUT_DIRECTORY ${EXAMPLE_NAME})
	target_link_libraries(${target} ${LIBRPMA_LIBRARIES} ${LIBRT_LIBRARIES}
		${LIBIBVERBS_LIBRARIES})
	target_include_directories(${target} PRIVATE common
		${LIBRPMA_SOURCE_DIR} ${LIBIBVERBS_INCLUDE_DIRS})

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

add_example(NAME 01-connection BIN server
	SRCS 01-connection/server.c)
add_example(NAME 01-connection BIN client
	SRCS 01-connection/client.c)
add_example(NAME 02-read-to-volatile BIN server
	SRCS 02-read-to-volatile/server.c)
add_example(NAME 02-read-to-volatile BIN client
	SRCS 02-read-to-volatile/client.c)
add_example(NAME 03-read-to-persistent BIN server
	SRCS 03-read-to-persistent/server.c)
add_example(NAME 03-read-to-persistent BIN client
	SRCS 03-read-to-persistent/client.c common/common-hello.c common/common-utils.c)
add_example(NAME 04-write-to-persistent BIN server
	SRCS 04-write-to-persistent/server.c common/common-hello.c)
add_example(NAME 04-write-to-persistent BIN client
	SRCS 04-write-to-persistent/client.c common/common-hello.c common/common-utils.c)
add_example(NAME 05-flush-to-persistent BIN server
	SRCS 05-flush-to-persistent/server.c common/common-hello.c)
add_example(NAME 05-flush-to-persistent BIN client
	SRCS 05-flush-to-persistent/client.c common/common-hello.c common/common-utils.c)
add_example(NAME 06-multiple-connections BIN server
	SRCS 06-multiple-connections/server.c common/common-epoll.c)
add_example(NAME 06-multiple-connections BIN client
	SRCS 06-multiple-connections/client.c)
add_example(NAME 06scch-multiple-connections BIN server
	SRCS 06scch-multiple-connections/server.c common/common-epoll.c)
add_example(NAME 06scch-multiple-connections BIN client
	SRCS 06scch-multiple-connections/client.c)
add_example(NAME 07-atomic-write BIN server
	SRCS 07-atomic-write/server.c)
add_example(NAME 07-atomic-write BIN client
	SRCS 07-atomic-write/client.c)
add_example(NAME 08-messages-ping-pong BIN server
	SRCS 08-messages-ping-pong/server.c common/common-messages-ping-pong.c)
add_example(NAME 08-messages-ping-pong BIN client
	SRCS 08-messages-ping-pong/client.c common/common-messages-ping-pong.c common/common-utils.c)
add_example(NAME 08srq-simple-messages-ping-pong-with-srq BIN server
	SRCS 08srq-simple-messages-ping-pong-with-srq/server.c common/common-messages-ping-pong.c)
add_example(NAME 08srq-simple-messages-ping-pong-with-srq BIN client
	SRCS 08srq-simple-messages-ping-pong-with-srq/client.c common/common-messages-ping-pong.c common/common-utils.c)
add_example(NAME 09-flush-to-persistent-GPSPM BIN server USE_LIBPROTOBUFC
	SRCS 09-flush-to-persistent-GPSPM/server.c
	common/gpspm/GPSPM_flush.pb-c.c common/common-hello.c)
add_example(NAME 09-flush-to-persistent-GPSPM BIN client USE_LIBPROTOBUFC
	SRCS 09-flush-to-persistent-GPSPM/client.c
	common/gpspm/GPSPM_flush.pb-c.c common/common-hello.c common/common-utils.c)
add_example(NAME 09scch-flush-to-persistent-GPSPM BIN server USE_LIBPROTOBUFC
	SRCS 09scch-flush-to-persistent-GPSPM/server.c
	common/gpspm/GPSPM_flush.pb-c.c common/common-hello.c)
add_example(NAME 09scch-flush-to-persistent-GPSPM BIN client USE_LIBPROTOBUFC
	SRCS 09scch-flush-to-persistent-GPSPM/client.c
	common/gpspm/GPSPM_flush.pb-c.c common/common-hello.c common/common-utils.c)
add_example(NAME 10-send-with-imm BIN server
	SRCS 10-send-with-imm/server.c)
add_example(NAME 10-send-with-imm BIN client
	SRCS 10-send-with-imm/client.c)
add_example(NAME 11-write-with-imm BIN server
	SRCS 11-write-with-imm/server.c)
add_example(NAME 11-write-with-imm BIN client
	SRCS 11-write-with-imm/client.c)
add_example(NAME 12-receive-completion-queue BIN server
	SRCS 12-receive-completion-queue/server.c
	12-receive-completion-queue/receive-completion-queue-common.c)
add_example(NAME 12-receive-completion-queue BIN client
	SRCS 12-receive-completion-queue/client.c
	12-receive-completion-queue/receive-completion-queue-common.c common/common-utils.c)
add_example(NAME 12scch-receive-completion-queue BIN server
	SRCS 12scch-receive-completion-queue/server.c)
add_example(NAME 12scch-receive-completion-queue BIN client
	SRCS 12scch-receive-completion-queue/client.c common/common-utils.c)
add_example(NAME 13-messages-ping-pong-with-srq BIN server
	SRCS 13-messages-ping-pong-with-srq/server.c common/common-epoll.c)
add_example(NAME 13-messages-ping-pong-with-srq BIN client
	SRCS 13-messages-ping-pong-with-srq/client.c common/common-messages-ping-pong.c common/common-utils.c)

add_example(NAME log BIN log SRCS
	log/log-example.c
	log/log-worker.c
	${LIBRPMA_SOURCE_DIR}/log.c
	${LIBRPMA_SOURCE_DIR}/log_default.c)
