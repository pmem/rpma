#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2020-2022, Intel Corporation
#

include(../../cmake/ctest_helpers.cmake)

#
# Undefine _FORTIFY_SOURCE in the Release build.
# _FORTIFY_SOURCE replaces original '*printf' functions
# and causes cmocka mocks of those functions to not work.
#
add_flag("-U_FORTIFY_SOURCE" RELEASE)

function(build_log_default name)
	build_test_src(UNIT NAME ut-log_default-${name} SRCS
		${name}.c
		${TEST_UNIT_COMMON_DIR}/mocks-rpma-log.c
		${TEST_UNIT_COMMON_DIR}/mocks-stdio.c
		${TEST_UNIT_COMMON_DIR}/mocks-syslog.c
		${TEST_UNIT_COMMON_DIR}/mocks-time.c
		${TEST_UNIT_COMMON_DIR}/mocks-glibc.c
		${LIBRPMA_SOURCE_DIR}/log_default.c)

		set_target_properties(ut-log_default-${name} PROPERTIES
			LINK_FLAGS
			"-Wl,--wrap=vsnprintf,--wrap=snprintf,--wrap=fprintf,--wrap=clock_gettime,--wrap=localtime_r,--wrap=strftime,--wrap=syscall")

	add_test_generic(NAME ut-log_default-${name} TRACERS none)
endfunction()

build_log_default(init-fini)
build_log_default(function)
