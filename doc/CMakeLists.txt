#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2019-2020, Intel Corporation
#

find_program(TXT2MAN NAMES txt2man)
if(TXT2MAN)
	# create man pages from sources
	execute_process(COMMAND ${CMAKE_SOURCE_DIR}/utils/src2mans.sh ${CMAKE_SOURCE_DIR}/src)

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
	message(WARNING "txt2man not found - man pages will not be generated")
endif()
