#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2020, Intel Corporation
#

if(NOT LIBIBVERBS_FOUND)
	find_package(LIBIBVERBS REQUIRED libibverbs)
endif()

if(NOT LIBRDMACM_FOUND)
	find_package(LIBRDMACM REQUIRED librdmacm)
endif()

message(STATUS "Checking for module 'librpma' w/o PkgConfig")

find_library(LIBRPMA_LIBRARY NAMES librpma.so librpma rpma)
set(LIBRPMA_LIBRARIES ${LIBRPMA_LIBRARY})

if(LIBRPMA_LIBRARY)
	message(STATUS "  Found librpma w/o PkgConfig")
else()
	set(MSG_NOT_FOUND "librpma NOT found (set CMAKE_PREFIX_PATH to point the location)")
	if(LIBRPMA_FIND_REQUIRED)
		message(FATAL_ERROR ${MSG_NOT_FOUND})
	else()
		message(WARNING ${MSG_NOT_FOUND})
	endif()
endif()
