#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2020-2021, Intel Corporation
#

message(STATUS "Checking for module 'libpmem' w/o PkgConfig")

find_library(LIBPMEM_LIBRARY NAMES libpmem.so libpmem pmem)
set(LIBPMEM_LIBRARIES ${LIBPMEM_LIBRARY})

if(LIBPMEM_LIBRARY)
	message(STATUS "  Found libpmem w/o PkgConfig")
else()
	set(MSG_NOT_FOUND "libpmem NOT found (set CMAKE_PREFIX_PATH to point the location)")
	if(LIBPMEM_FIND_REQUIRED)
		message(FATAL_ERROR ${MSG_NOT_FOUND})
	else()
		message(WARNING ${MSG_NOT_FOUND})
	endif()
endif()
