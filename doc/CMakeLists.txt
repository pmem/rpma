#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2019-2021, Intel Corporation
#

find_program(SRC2MAN NAMES src2man)
find_program(GROFF NAMES groff)
find_program(FIND NAMES find)
find_program(DIFF NAMES diff)
find_program(GAWK NAMES gawk) # required by src2man

find_program(PANDOC NAMES pandoc)
if(NOT PANDOC)
	message(WARNING "pandoc not found - Markdown documentation will not be generated")
endif()

if(SRC2MAN AND GROFF AND FIND AND DIFF AND GAWK)
	# create man pages from sources
	set(man3_list ${CMAKE_SOURCE_DIR}/doc/manuals_3.txt)
	set(man7_list ${CMAKE_SOURCE_DIR}/doc/manuals_7.txt)

	add_custom_target(doc ALL 
		COMMAND ${CMAKE_SOURCE_DIR}/utils/src2mans.sh
			${CMAKE_SOURCE_DIR}/src ${man3_list} ${man7_list}
			${CMAKE_SOURCE_DIR}/utils/mans_header.md)

	add_custom_target(doc-fix
		COMMAND ${CMAKE_SOURCE_DIR}/utils/src2mans.sh
			${CMAKE_SOURCE_DIR}/src ${man3_list} ${man7_list}
			${CMAKE_SOURCE_DIR}/utils/mans_header.md
			fix)

	file(STRINGS ${man3_list} man3)
	file(STRINGS ${man7_list} man7)

	#
	# It is just:
	#    list(TRANSFORM man3 PREPEND "${CMAKE_CURRENT_BINARY_DIR}/")
	# but 'list(TRANSFORM' requires CMake>=v3.12
	#
	set(new_man3 "")
	foreach(item IN LISTS man3)
		set(new_man3 "${CMAKE_CURRENT_BINARY_DIR}/${item};${new_man3}")
	endforeach()

	set(new_man7 "")
	foreach(item IN LISTS man7)
		set(new_man7 "${CMAKE_CURRENT_BINARY_DIR}/${item};${new_man7}")
	endforeach()

	# install manpages
	install(FILES ${new_man3}
		DESTINATION ${CMAKE_INSTALL_MANDIR}/man3)
	install(FILES ${new_man7}
		DESTINATION ${CMAKE_INSTALL_MANDIR}/man7)
else()
	if(NOT SRC2MAN)
		message(WARNING "src2man not found - man pages will not be generated")
	endif()
	if(NOT GROFF)
		message(WARNING "groff not found - man pages will not be generated")
	endif()
	if(NOT FIND)
		message(WARNING "find not found - man pages will not be generated")
	endif()
	if(NOT DIFF)
		message(WARNING "diff not found - man pages will not be generated")
	endif()
	if(NOT GAWK)
		message(WARNING "gawk not found - man pages will not be generated")
	endif()
endif()
