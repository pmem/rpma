#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2020-2022, Intel Corporation
# Copyright 2021, Fujitsu
#

cmake_minimum_required(VERSION 3.3)
project(flush-to-persistent C)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}
	${CMAKE_SOURCE_DIR}/../cmake
	${CMAKE_SOURCE_DIR}/../../cmake)

include(${CMAKE_SOURCE_DIR}/../cmake/common.cmake)
include(${CMAKE_SOURCE_DIR}/../../cmake/functions.cmake)

# check if all required IBV_ADVISE_MR* flags are supported
are_ibv_advise_flags_supported(IBV_ADVISE_MR_FLAGS_SUPPORTED)

# set LIBRT_LIBRARIES if linking with librt is required
check_if_librt_is_required()

set(LIBPMEM_REQUIRED_VERSION 1.6)

find_package(PkgConfig QUIET)

if(PKG_CONFIG_FOUND)
	pkg_check_modules(LIBRPMA librpma)
	pkg_check_modules(LIBIBVERBS libibverbs)
	pkg_check_modules(LIBPROTOBUFC libprotobuf-c)
	pkg_check_modules(LIBPMEM QUIET libpmem>=${LIBPMEM_REQUIRED_VERSION})
endif()
if(NOT LIBRPMA_FOUND)
	find_package(LIBRPMA REQUIRED librpma)
endif()
if(NOT LIBIBVERBS_FOUND)
	find_package(LIBIBVERBS REQUIRED libibverbs)
endif()
if(NOT LIBPROTOBUFC_FOUND)
	find_package(LIBPROTOBUFC REQUIRED libprotobuf-c)
endif()
if(NOT LIBPMEM_FOUND)
	find_package(LIBPMEM ${LIBPMEM_REQUIRED_VERSION} QUIET)
endif()

link_directories(${LIBRPMA_LIBRARY_DIRS})

add_example_with_pmem(NAME server USE_LIBPROTOBUFC SRCS server.c ../common/gpspm/GPSPM_flush.pb-c.c)
add_example_with_pmem(NAME client USE_LIBPROTOBUFC SRCS client.c ../common/gpspm/GPSPM_flush.pb-c.c)
