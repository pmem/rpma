#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2020, Intel Corporation
#

message(STATUS "Checking for module 'libprotobuf-c' w/o PkgConfig")

find_library(LIBPROTOBUFC_LIBRARY NAMES libprotobuf-c.so libprotobuf-c protobuf-c)
set(LIBPROTOBUFC_LIBRARIES ${LIBPROTOBUFC_LIBRARY})

if(LIBPROTOBUFC_LIBRARY)
	message(STATUS "  Found libprotobuf-c w/o PkgConfig")
else()
	set(MSG_NOT_FOUND "libprotobuf-c NOT found (set CMAKE_PREFIX_PATH to point the location)")
	if(LIBPROTOBUFC_FIND_REQUIRED)
		message(FATAL_ERROR ${MSG_NOT_FOUND})
	else()
		message(WARNING ${MSG_NOT_FOUND})
	endif()
endif()
