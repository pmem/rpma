#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2020, Intel Corporation
#

include(../cmake/ctest_helpers.cmake)

add_flag(-DUNIT_TESTING=1)

build_test_src(log-test
       log-test.c
       log-test-common.c
       ${LIBRPMA_SOURCE_DIR}/common/log.c
       )
set_target_properties(log-test PROPERTIES 
			COMPILE_FLAGS -DRPMA_LOG_INIT_AT_STARTUP_SUSPENDED)

add_test_generic(NAME log-test TRACERS none)


build_test_src(log-test-enabled
       log-test-enabled.c
       log-test-common.c
       ${LIBRPMA_SOURCE_DIR}/common/log.c
       )


add_test_generic(NAME log-test-enabled TRACERS none)
