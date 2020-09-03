#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2020, Intel Corporation
#

message(STATUS "Checking for module 'librdmacm>=${LIBRDMACM_REQUIRED_VERSION}'")

find_library(LIBRDMACM_LIBRARY NAMES librdmacm.so librdmacm rdmacm)
set(LIBRDMACM_LIBRARIES ${LIBRDMACM_LIBRARY})

if(LIBRDMACM_LIBRARY)
	message(STATUS "  Found librdmacm w/o PkgConfig")
else()
	set(MSG_NOT_FOUND "librdmacm NOT found (set CMAKE_PREFIX_PATH to point the location)")
	if(LIBRDMACM_FIND_REQUIRED)
		message(FATAL_ERROR ${MSG_NOT_FOUND})
	else()
		message(WARNING ${MSG_NOT_FOUND})
	endif()
endif()
