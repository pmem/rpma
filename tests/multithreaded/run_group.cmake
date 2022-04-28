#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021-2022, Intel Corporation
#

include(${SRC_DIR}/../../cmake/helpers.cmake)

setup()

# XXX change ulimits to enable 32 threads
# set(THREADS 32)
set(THREADS 7)

if("$ENV{RPMA_SOFT_ROCE_IP}" STREQUAL "")
	set(SOFT_ROCE_IP "127.0.0.1")
else()
	set(SOFT_ROCE_IP $ENV{RPMA_SOFT_ROCE_IP})
endif()

if("$ENV{RPMA_SOFT_ROCE_PORT}" STREQUAL "")
	set(SOFT_ROCE_PORT "7204")
else()
	set(SOFT_ROCE_PORT $ENV{RPMA_SOFT_ROCE_PORT})
endif()

execute(${TEST_EXECUTABLE} ${THREADS} ${SOFT_ROCE_IP} ${SOFT_ROCE_PORT})

finish()
