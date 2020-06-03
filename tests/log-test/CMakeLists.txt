#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2020, Intel Corporation
#

include(../cmake/ctest_helpers.cmake)

add_flag(-DUNIT_TESTING=1)

build_test_src(log-test
       log-test.c
       log-test-librpma_log.c
       #${LIBRPMA_SOURCE_DIR}/common/librpma_log.c
       )

#target_link_libraries(log-test test-rpma-utils)

#set_target_properties(log-test
#       PROPERTIES
#       LINK_FLAGS "-Wl,--wrap=_test_malloc")

add_test_generic(NAME log-test TRACERS none)
