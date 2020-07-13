#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2020, Intel Corporation
#

include(../../cmake/ctest_helpers.cmake)

build_test_src(log-test
	log-test.c
	log-test-common.c
	${LIBRPMA_SOURCE_DIR}/common/log.c)

set_target_properties(log-test PROPERTIES LINK_FLAGS "-Wl,--wrap=fprintf")

add_test_generic(NAME log-test TRACERS none)

build_test_src(log-test-disabled-at-startup
	log-test-disabled.c
	log-test-common.c
	${LIBRPMA_SOURCE_DIR}/common/log.c)

set_target_properties(log-test-disabled-at-startup PROPERTIES 
	COMPILE_FLAGS -DRPMA_LOG_INIT_DEFAULT_OFF)

add_test_generic(NAME log-test-disabled-at-startup TRACERS none)
