#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2020-2022, Intel Corporation
# Copyright 2021-2022, Fujitsu
#

add_cstyle(src
	${CMAKE_CURRENT_SOURCE_DIR}/*.[ch]
	${CMAKE_CURRENT_SOURCE_DIR}/include/*.h)

add_check_whitespace(src
	${CMAKE_CURRENT_SOURCE_DIR}/*.[ch]
	${CMAKE_CURRENT_SOURCE_DIR}/include/*.h)

set(SOURCES
	conn.c
	conn_cfg.c
	conn_req.c
	cq.c
	debug.c
	ep.c
	flush.c
	info.c
	librpma.c
	log.c
	log_default.c
	mr.c
	peer.c
	peer_cfg.c
	private_data.c
	rpma_err.c
	utils.c
	srq.c
	srq_cfg.c)

add_library(rpma SHARED ${SOURCES})

target_include_directories(rpma PRIVATE . include)

target_link_libraries(rpma PRIVATE
	${LIBIBVERBS_LIBRARIES}
	${LIBRDMACM_LIBRARIES}
	-Wl,--version-script=${CMAKE_SOURCE_DIR}/src/librpma.map)

set_target_properties(rpma PROPERTIES
	SOVERSION 0
	PUBLIC_HEADER "include/librpma.h")

target_compile_definitions(rpma PRIVATE SRCVERSION="${SRCVERSION}")

if(DEBUG_LOG_TRACE)
	target_compile_definitions(rpma PRIVATE DEBUG_LOG_TRACE=1)
endif()

if(DEBUG_FAULT_INJECTION)
	target_compile_definitions(rpma PRIVATE DEBUG_FAULT_INJECTION=1)
endif()

if(VALGRIND_FOUND)
	target_include_directories(rpma PRIVATE src/valgrind)
endif()

if(IBV_ADVISE_MR_FLAGS_SUPPORTED)
	target_compile_definitions(rpma PRIVATE IBV_ADVISE_MR_FLAGS_SUPPORTED=1)
endif()

install(TARGETS rpma
	PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
