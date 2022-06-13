#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021-2022, Intel Corporation
#

include(${SRC_DIR}/../../cmake/helpers.cmake)

setup()

if(TRACER STREQUAL "memcheck")
	set(THREADS 2)
else()
	set(THREADS 32)
endif()

message(NOTICE "TRACER=${TRACER}")
message(NOTICE "THREADS=${THREADS}")

if("$ENV{RPMA_TESTING_IP}" STREQUAL "")
	set(TESTING_IP "127.0.0.1")
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
