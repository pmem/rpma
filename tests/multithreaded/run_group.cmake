#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021-2022, Intel Corporation
#

include(${SRC_DIR}/../../cmake/helpers.cmake)

setup()

if(NOT TRACER STREQUAL "none")
	message(NOTICE "Valgrind tool: ${TRACER}")
endif()

if(TRACER STREQUAL "memcheck")
	set(THREADS 2)
else()
	set(THREADS ${MAX_THREADS})
endif()

message(NOTICE "Number of threads: ${THREADS}")

if("$ENV{RPMA_TESTING_IP}" STREQUAL "")
	message(FATAL_ERROR "RPMA_TESTING_IP is not set!")
else()
	set(TESTING_IP $ENV{RPMA_TESTING_IP})
endif()

if("$ENV{RPMA_TESTING_PORT}" STREQUAL "")
	set(TESTING_PORT "7204")
else()
	set(TESTING_PORT $ENV{RPMA_TESTING_PORT})
endif()

execute(${TEST_EXECUTABLE} ${THREADS} ${TESTING_IP} ${TESTING_PORT})

finish()
