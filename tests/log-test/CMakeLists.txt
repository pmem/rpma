#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2020, Intel Corporation
#

include(../cmake/ctest_helpers.cmake)

add_flag(-DUNIT_TESTING=1)

build_test_src(log-test
       log-test.c
       ${LIBRPMA_SOURCE_DIR}/common/log.c
       )

add_test_generic(NAME log-test TRACERS none)
