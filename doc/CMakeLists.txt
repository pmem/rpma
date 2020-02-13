#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2019-2020, Intel Corporation
#

add_check_whitespace(man
	${CMAKE_CURRENT_SOURCE_DIR}/*.md)

find_program(PANDOC NAMES pandoc)
if(PANDOC)
	# convert md files to manpage format
	add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/man/librpma.7
		MAIN_DEPENDENCY ${CMAKE_SOURCE_DIR}/doc/librpma.7.md
		COMMAND ${CMAKE_SOURCE_DIR}/utils/md2man/md2man.sh
			${CMAKE_SOURCE_DIR}/doc/librpma.7.md
			${CMAKE_SOURCE_DIR}/utils/md2man/default.man
			${CMAKE_BINARY_DIR}/man/librpma.7
			${VERSION})

	# install manpages
	install(FILES ${CMAKE_BINARY_DIR}/man/librpma.7
		DESTINATION ${CMAKE_INSTALL_MANDIR}/man7)
else()
	message(WARNING "pandoc not found - man pages will not be generated")
endif()

include(FindDoxygen)
if(DOXYGEN_FOUND AND DOXYGEN_DOT_FOUND)
	configure_file("${CMAKE_CURRENT_SOURCE_DIR}/librpma.Doxyfile.in"
		"${CMAKE_CURRENT_BINARY_DIR}/librpma.Doxyfile" @ONLY)
	install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/cpp_html/ DESTINATION ${CMAKE_INSTALL_DOCDIR})
elseif(NOT DOXYGEN_FOUND)
	message(WARNING "Doxygen not found - Doxygen documentation will not be generated")
else()
	message(WARNING "Dot tool not found - Doxygen documentation will not be generated")
endif()

if(PANDOC AND DOXYGEN_FOUND AND DOXYGEN_DOT_FOUND)
	add_custom_target(doc ALL
		${DOXYGEN_EXECUTABLE} "${CMAKE_CURRENT_BINARY_DIR}/librpma.Doxyfile"
		DEPENDS ${CMAKE_BINARY_DIR}/man/librpma.7
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
elseif(PANDOC)
	add_custom_target(doc ALL
		DEPENDS ${CMAKE_BINARY_DIR}/man/librpma.7
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
elseif(DOXYGEN_FOUND AND DOXYGEN_DOT_FOUND)
	add_custom_target(doc ALL
		${DOXYGEN_EXECUTABLE} "${CMAKE_CURRENT_BINARY_DIR}/librpma.Doxyfile"
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
endif()
