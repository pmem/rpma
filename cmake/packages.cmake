#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2018-2020, Intel Corporation
#

#
# packages.cmake - CPack configuration for rpm and deb generation
#

string(TOUPPER "${CPACK_GENERATOR}" CPACK_GENERATOR)

if(NOT ("${CPACK_GENERATOR}" STREQUAL "DEB" OR
	"${CPACK_GENERATOR}" STREQUAL "RPM"))
	message(FATAL_ERROR "Wrong CPACK_GENERATOR value, valid generators are: DEB, RPM")
endif()

set(CPACK_PACKAGING_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")
set(CMAKE_INSTALL_TMPDIR /tmp CACHE PATH "Output dir for tmp")
set(CPACK_COMPONENTS_ALL_IN_ONE)

# Filter out some of directories from %dir section, which are expected
# to exist in filesystem. Leaving them might lead to conflicts with other
# packages (for example with 'filesystem' package on fedora which specify
# /usr, /usr/local, etc.)
set(CPACK_RPM_EXCLUDE_FROM_AUTO_FILELIST_ADDITION
	${CPACK_PACKAGING_INSTALL_PREFIX}
	${CPACK_PACKAGING_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}
	${CPACK_PACKAGING_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}/pkgconfig
	${CPACK_PACKAGING_INSTALL_PREFIX}/${CMAKE_INSTALL_INCDIR}
	${CPACK_PACKAGING_INSTALL_PREFIX}/share
	${CPACK_PACKAGING_INSTALL_PREFIX}/share/doc)

set(CPACK_PACKAGE_NAME "librpma")
set(CPACK_PACKAGE_VERSION ${VERSION})
set(CPACK_PACKAGE_VERSION_MAJOR ${VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${VERSION_MINOR})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "library to simplify accessing persistent memory devices on remote hosts over RDMA")
set(CPACK_PACKAGE_VENDOR "Intel")

set(CPACK_RPM_PACKAGE_NAME "librpma-devel")
set(CPACK_RPM_PACKAGE_GROUP "Development/Libraries")
set(CPACK_RPM_PACKAGE_LICENSE "BSD")
set(CPACK_RPM_PACKAGE_ARCHITECTURE x86_64)
set(CPACK_RPM_PACKAGE_REQUIRES "libibverbs >= ${LIBIBVERBS_REQUIRED_VERSION}, librdmacm >= ${LIBRDMACM_REQUIRED_VERSION}")
#set(CPACK_RPM_CHANGELOG_FILE ${CMAKE_SOURCE_DIR}/ChangeLog)

set(CPACK_DEBIAN_PACKAGE_NAME "librpma-dev")
set(CPACK_DEBIAN_PACKAGE_VERSION ${CPACK_PACKAGE_VERSION})
set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE amd64)
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libibverbs-dev (>= ${LIBIBVERBS_REQUIRED_VERSION}), librdmacm-dev (>= ${LIBRDMACM_REQUIRED_VERSION})")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "tomasz.gromadzki@intel.com")

if("${CPACK_GENERATOR}" STREQUAL "RPM")
	set(CPACK_PACKAGE_FILE_NAME
		${CPACK_RPM_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}.${CPACK_RPM_PACKAGE_ARCHITECTURE})
elseif("${CPACK_GENERATOR}" STREQUAL "DEB")
	# We are using "gnutar" to avoid this bug:
	# https://gitlab.kitware.com/cmake/cmake/issues/14332
	set(CPACK_DEBIAN_ARCHIVE_TYPE "gnutar")
	set(CPACK_PACKAGE_FILE_NAME
		${CPACK_DEBIAN_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}_${CPACK_DEBIAN_PACKAGE_ARCHITECTURE})
endif()

set(targetDestDir ${CMAKE_INSTALL_TMPDIR})
include(CPack)
