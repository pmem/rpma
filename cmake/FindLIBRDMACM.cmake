#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2020, Intel Corporation
#

message(STATUS "Looking for librdmacm ...")

find_library(LIBRDMACM_LIBRARY
	NAMES librdmacm.so librdmacm.so.1
	PATHS /usr/lib64 /usr/lib)

find_path(LIBRDMACM_INCLUDE_DIR
	NAMES rdma_cma.h
	PATHS /usr/include/rdma)

set(LIBRDMACM_LIBRARIES ${LIBRDMACM_LIBRARY})
set(LIBRDMACM_INCLUDE_DIRS ${LIBRDMACM_INCLUDE_DIR})

set(MSG_NOT_FOUND "librdmacm NOT found (set CMAKE_PREFIX_PATH to point the location)")
if(NOT (LIBRDMACM_LIBRARY AND LIBRDMACM_INCLUDE_DIR))
	if(LIBRDMACM_FIND_REQUIRED)
		message(FATAL_ERROR ${MSG_NOT_FOUND})
	else()
		message(WARNING ${MSG_NOT_FOUND})
	endif()
else()
	message(STATUS "  Found librdmacm library: ${LIBRDMACM_LIBRARY}")
	message(STATUS "  Found librdmacm include directory: ${LIBRDMACM_INCLUDE_DIR}")
endif()

mark_as_advanced(LIBRDMACM_LIBRARY LIBRDMACM_INCLUDE_DIR)
