#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2020, Intel Corporation
#

include(../../cmake/ctest_helpers.cmake)

function(build_log_default name)
	build_test_src(UNIT NAME log_default-${name} SRCS
		${name}.c
		${TEST_UNIT_COMMON_DIR}/mocks-rpma-log.c
		${TEST_UNIT_COMMON_DIR}/mocks-stdio.c
		${TEST_UNIT_COMMON_DIR}/mocks-syslog.c
		${TEST_UNIT_COMMON_DIR}/mocks-time.c
		${LIBRPMA_SOURCE_DIR}/log_default.c)

		set_target_properties(log_default-${name} PROPERTIES
			LINK_FLAGS
			"-Wl,--wrap=vsnprintf,--wrap=snprintf,--wrap=fprintf,--wrap=clock_gettime,--wrap=localtime,--wrap=strftime")

	add_test_generic(NAME log_default-${name} TRACERS none)
endfunction()

build_log_default(init-fini)
build_log_default(function)