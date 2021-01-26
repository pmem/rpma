#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021, Intel Corporation
#

include(${SRC_DIR}/../../cmake/helpers.cmake)

setup()

set(THREADS 32)

if("$ENV{RPMA_SOFT_ROCE_IP}" STREQUAL "")
	set(SOFT_ROCE_IP "127.0.0.1")
else()
	set(SOFT_ROCE_IP $ENV{RPMA_SOFT_ROCE_IP})
endif()

execute(${TEST_EXECUTABLE} ${THREADS} ${SOFT_ROCE_IP})

finish()
