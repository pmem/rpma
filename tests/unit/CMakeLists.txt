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

add_subdirectory(conn-test)
add_subdirectory(conn_req-test)
add_subdirectory(ep-test)
add_subdirectory(error-test)
add_subdirectory(info-test)
add_subdirectory(mr-test)
add_subdirectory(peer-test)
add_subdirectory(private_data-test)
add_subdirectory(template-test)
add_subdirectory(utils-test)