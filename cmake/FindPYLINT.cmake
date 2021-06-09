#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021, Intel Corporation
#

message(STATUS "Checking for executable 'pylint'")

find_program(PYLINT_EXECUTABLE NAMES pylint3 pylint-3 pylint)

if(PYLINT_EXECUTABLE)
	message(STATUS "  Found pylint")
else()
	set(MSG_NOT_FOUND "pylint NOT found (set CMAKE_PREFIX_PATH to point the location)")
	if(PYLINT_FIND_REQUIRED)
		message(FATAL_ERROR ${MSG_NOT_FOUND})
	else()
		message(WARNING ${MSG_NOT_FOUND})
	endif()
endif()
