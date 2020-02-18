#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2020, Intel Corporation
#

add_cstyle(src
	${CMAKE_CURRENT_SOURCE_DIR}/*.[ch]
	${CMAKE_CURRENT_SOURCE_DIR}/core/*.[ch]
	${CMAKE_CURRENT_SOURCE_DIR}/include/*.h)

add_check_whitespace(src
	${CMAKE_CURRENT_SOURCE_DIR}/*.[ch]
	${CMAKE_CURRENT_SOURCE_DIR}/core/*.[ch]
	${CMAKE_CURRENT_SOURCE_DIR}/include/*.h)

set(SOURCES
	core/alloc.c
	core/os_posix.c
	core/os_thread_posix.c
	core/out.c
	core/ravl.c
	core/util.c
	core/util_posix.c
	config.c
	rpma_utils.c)

add_library(rpma SHARED ${SOURCES})

target_include_directories(rpma PRIVATE . include core)

if(TESTS_USE_FAULT_INJECTION)
	target_compile_definitions(rpma PUBLIC FAULT_INJECTION=1)
endif()

target_link_libraries(rpma PRIVATE
	${LIBPMEM_LIBRARIES}
	${LIBIBVERBS_LIBRARIES}
	${LIBRDMACM_LIBRARIES}
	-Wl,--version-script=${CMAKE_SOURCE_DIR}/src/librpma.map)

set_target_properties(rpma PROPERTIES
	SOVERSION 0
	PUBLIC_HEADER include/librpma.h
	PUBLIC_HEADER include/base.h
	PUBLIC_HEADER include/msg.h
	PUBLIC_HEADER include/rma.h)

target_compile_definitions(rpma PRIVATE SRCVERSION="${SRCVERSION}")

if(VALGRIND_FOUND)
	target_include_directories(rpma PRIVATE src/valgrind)
	# Enable librpma valgrind annotations
	target_compile_options(rpma PRIVATE -DVALGRIND_ENABLED=1)
endif()

install(TARGETS rpma
	PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
