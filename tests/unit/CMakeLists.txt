#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2020, Intel Corporation
# Copyright 2021, Fujitsu
#

add_subdirectory(conn)
add_subdirectory(conn_cfg)
add_subdirectory(conn_req)
add_subdirectory(cq)
add_subdirectory(ep)
add_subdirectory(error)
add_subdirectory(flush)
add_subdirectory(info)
add_subdirectory(librpma_constructor)
add_subdirectory(log)
add_subdirectory(mr)
add_subdirectory(peer)
add_subdirectory(peer_cfg)
add_subdirectory(private_data)
add_subdirectory(template)
add_subdirectory(utils)

if(TESTS_NO_FORTIFY_SOURCE)
	add_subdirectory(log_default)
endif()
