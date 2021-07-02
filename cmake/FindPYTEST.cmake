#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021, Intel Corporation
#

message(STATUS "Checking for executable 'pytest'")

find_program(PYTEST_EXECUTABLE NAMES pytest-3 pytest)

if(PYTEST_EXECUTABLE)
	message(STATUS "  Found pytest")
else()
	set(MSG_NOT_FOUND "pytest NOT found (set CMAKE_PREFIX_PATH to point the location)")
	if(PYTEST_FIND_REQUIRED)
		message(FATAL_ERROR ${MSG_NOT_FOUND})
	else()
		message(WARNING ${MSG_NOT_FOUND})
	endif()
endif()
