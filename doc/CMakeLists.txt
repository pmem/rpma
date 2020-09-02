#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2019-2020, Intel Corporation
#

find_program(TXT2MAN NAMES txt2man)
find_program(GROFF NAMES groff)

if(TXT2MAN AND GROFF)
	# create man pages from sources
	set(man3_list ${CMAKE_SOURCE_DIR}/doc/manuals_3.txt)
	set(man7_list ${CMAKE_SOURCE_DIR}/doc/manuals_7.txt)

	add_custom_target(doc ALL 
		COMMAND ${CMAKE_SOURCE_DIR}/utils/src2mans.sh
			${CMAKE_SOURCE_DIR}/src ${man3_list} ${man7_list})

	add_custom_target(doc-fix
		COMMAND ${CMAKE_SOURCE_DIR}/utils/src2mans.sh
			${CMAKE_SOURCE_DIR}/src ${man3_list} ${man7_list} fix)

	file(STRINGS ${man3_list} man3)
	file(STRINGS ${man7_list} man7)

	list(TRANSFORM man3 PREPEND "${CMAKE_CURRENT_BINARY_DIR}/")
	list(TRANSFORM man7 PREPEND "${CMAKE_CURRENT_BINARY_DIR}/")

	# install manpages
	install(FILES ${man7}
		DESTINATION ${CMAKE_INSTALL_MANDIR}/man7)
	install(FILES ${man3}
		DESTINATION ${CMAKE_INSTALL_MANDIR}/man3)
else()
	if(NOT TXT2MAN)
		message(WARNING "txt2man not found - man pages will not be generated")
	endif()
	if(NOT GROFF)
		message(WARNING "groff not found - man pages will not be generated")
	endif()
endif()
