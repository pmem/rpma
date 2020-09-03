#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2020, Intel Corporation
#

find_library(LIBRDMACM_LIBRARY NAMES librdmacm.so librdmacm rdmacm)
set(LIBRDMACM_LIBRARIES ${LIBRDMACM_LIBRARY})

set(MSG_NOT_FOUND "librdmacm NOT found (set CMAKE_PREFIX_PATH to point the location)")
if(NOT LIBRDMACM_LIBRARY)
	if(LIBRDMACM_FIND_REQUIRED)
		message(FATAL_ERROR ${MSG_NOT_FOUND})
	else()
		message(WARNING ${MSG_NOT_FOUND})
	endif()
endif()
