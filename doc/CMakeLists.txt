#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2019-2020, Intel Corporation
#

find_program(TXT2MAN NAMES txt2man)
find_program(GROFF NAMES groff)

if(TXT2MAN AND GROFF)
	# create man pages from sources
	add_custom_target(doc ALL 
		COMMAND ${CMAKE_SOURCE_DIR}/utils/src2mans.sh ${CMAKE_SOURCE_DIR}/src)

	file(GLOB_RECURSE man3
		${CMAKE_BINARY_DIR}/*.3)
	file(GLOB_RECURSE man7
		${CMAKE_BINARY_DIR}/*.7)
	
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
