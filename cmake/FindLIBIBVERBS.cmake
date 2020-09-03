#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2020, Intel Corporation
#

message(STATUS "Checking for module 'libibverbs>=${LIBIBVERBS_REQUIRED_VERSION}'")

find_library(LIBIBVERBS_LIBRARY NAMES libibverbs.so libibverbs ibverbs)
set(LIBIBVERBS_LIBRARIES ${LIBIBVERBS_LIBRARY})

if(LIBIBVERBS_LIBRARY)
	message(STATUS "  Found libibverbs w/o PkgConfig")
else()
	set(MSG_NOT_FOUND "libibverbs NOT found (set CMAKE_PREFIX_PATH to point the location)")
	if(LIBIBVERBS_FIND_REQUIRED)
		message(FATAL_ERROR ${MSG_NOT_FOUND})
	else()
		message(WARNING ${MSG_NOT_FOUND})
	endif()
endif()
