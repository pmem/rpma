#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2020, Intel Corporation
#

include(../cmake/ctest_helpers.cmake)

add_flag(-Wall)

if(USE_ASAN)
       add_sanitizer_flag(address)
endif()
if(USE_UBSAN)
       add_sanitizer_flag(undefined)
endif()

if(COVERAGE)
       set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -coverage")
endif()

find_packages()

# Integration tests break codecov results. The root cause is unknown.
# This condition is merely a WA for this issue.
if(NOT COVERAGE)
       add_subdirectory(example-01-connection)
endif()
