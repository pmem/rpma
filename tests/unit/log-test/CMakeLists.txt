#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2020, Intel Corporation
#

include(../../cmake/ctest_helpers.cmake)

build_test_src(UNIT NAME log-test SRCS
	log-test.c
	log-test-common.c
	log-test-to-stderr.c
	log-test-to-syslog.c
	log-test-to-custom-function.c
	${LIBRPMA_SOURCE_DIR}/log.c)

set_target_properties(log-test PROPERTIES 
	LINK_FLAGS "-Wl,--wrap=fprintf,--wrap=clock_gettime,--wrap=localtime,--wrap=strftime,--wrap=snprintf")

add_test_generic(NAME log-test TRACERS none)
