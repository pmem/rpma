#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2018-2020, Intel Corporation
#

add_custom_target(multithreaded)
include_directories(${LIBRPMA_INCLUDE_DIRS})
link_directories(${LIBRPMA_LIBRARY_DIRS})

add_subdirectory(utils)
