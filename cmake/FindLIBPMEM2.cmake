#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2020-2021, Intel Corporation
#

message(STATUS "Checking for module 'libpmem2' w/o PkgConfig")

find_library(LIBPMEM2_LIBRARY NAMES libpmem2.so libpmem2 pmem2)
set(LIBPMEM2_LIBRARIES ${LIBPMEM2_LIBRARY})

if(LIBPMEM2_LIBRARY)
	message(STATUS "  Found libpmem2 w/o PkgConfig")
else()
	set(MSG_NOT_FOUND "libpmem2 NOT found (set CMAKE_PREFIX_PATH to point the location)")
	if(LIBPMEM2_FIND_REQUIRED)
		message(FATAL_ERROR ${MSG_NOT_FOUND})
	else()
		message(WARNING ${MSG_NOT_FOUND})
	endif()
endif()
