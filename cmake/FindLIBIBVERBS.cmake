#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2020, Intel Corporation
#

message(STATUS "Looking for libibverbs ...")

find_library(LIBIBVERBS_LIBRARY
	NAMES libibverbs.so libibverbs.so.1
	PATHS /usr/lib64 /usr/lib)

find_path(LIBIBVERBS_INCLUDE_DIR
	NAMES verbs.h
	PATHS /usr/include/infiniband)

set(LIBIBVERBS_LIBRARIES ${LIBIBVERBS_LIBRARY})
set(LIBIBVERBS_INCLUDE_DIRS ${LIBIBVERBS_INCLUDE_DIR})

set(MSG_NOT_FOUND "libibverbs NOT found (set CMAKE_PREFIX_PATH to point the location)")
if(NOT (LIBIBVERBS_LIBRARY AND LIBIBVERBS_INCLUDE_DIR))
	if(LIBIBVERBS_FIND_REQUIRED)
		message(FATAL_ERROR ${MSG_NOT_FOUND})
	else()
		message(WARNING ${MSG_NOT_FOUND})
	endif()
else()
	message(STATUS "  Found libibverbs library: ${LIBIBVERBS_LIBRARY}")
	message(STATUS "  Found libibverbs include directory: ${LIBIBVERBS_INCLUDE_DIR}")
endif()

mark_as_advanced(LIBIBVERBS_LIBRARY LIBIBVERBS_INCLUDE_DIR)
