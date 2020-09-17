#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2018-2020, Intel Corporation
#

#
# functions.cmake - helper functions for CMakeLists.txt
#

# prepends prefix to list of strings
function(prepend var prefix)
	set(listVar "")
	foreach(f ${ARGN})
		list(APPEND listVar "${prefix}/${f}")
	endforeach(f)
	set(${var} "${listVar}" PARENT_SCOPE)
endfunction()

# Checks whether flag is supported by current C compiler and appends
# it to the relevant cmake variable.
# 1st argument is a flag
# 2nd (optional) argument is a build type (debug, release)
macro(add_flag flag)
	string(REPLACE - _ flag2 ${flag})
	string(REPLACE " " _ flag2 ${flag2})
	string(REPLACE = "_" flag2 ${flag2})
	set(check_name "C_HAS_${flag2}")

	check_c_compiler_flag(${flag} ${check_name})

	if (${${check_name}})
		if (${ARGC} EQUAL 1)
			set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${flag}")
		else()
			set(CMAKE_C_FLAGS_${ARGV1} "${CMAKE_C_FLAGS_${ARGV1}} ${flag}")
		endif()
	endif()
endmacro()

macro(add_sanitizer_flag flag)
	set(SAVED_CMAKE_REQUIRED_LIBRARIES ${CMAKE_REQUIRED_LIBRARIES})
	set(CMAKE_REQUIRED_LIBRARIES "${CMAKE_REQUIRED_LIBRARIES} -fsanitize=${flag}")

	check_c_compiler_flag("-fsanitize=${flag}" C_HAS_ASAN_UBSAN)
	if(C_HAS_ASAN_UBSAN)
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=${flag}")
		set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=${flag}")
	else()
		message("${flag} sanitizer not supported")
	endif()

	set(CMAKE_REQUIRED_LIBRARIES ${SAVED_CMAKE_REQUIRED_LIBRARIES})
endmacro()

# Generates cstyle-$name target and attaches it
# as a dependency of global "cstyle" target.
# cstyle-$name target verifies C style of files in current source dir.
# If more arguments are used, they are used as files to be checked
# instead.
# ${name} must be unique.
function(add_cstyle name)
	if(${ARGC} EQUAL 1)
		add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/cstyle-${name}-status
			DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/*.c
				${CMAKE_CURRENT_SOURCE_DIR}/*.h
			COMMAND ${PERL_EXECUTABLE}
				${CMAKE_SOURCE_DIR}/utils/cstyle -pP -o src2man
				${CMAKE_CURRENT_SOURCE_DIR}/*.c
				${CMAKE_CURRENT_SOURCE_DIR}/*.h
			COMMAND ${CMAKE_COMMAND} -E touch ${CMAKE_BINARY_DIR}/cstyle-${name}-status
			)
	else()
		add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/cstyle-${name}-status
			DEPENDS ${ARGN}
			COMMAND ${PERL_EXECUTABLE}
				${CMAKE_SOURCE_DIR}/utils/cstyle -pP -o src2man
				${ARGN}
			COMMAND ${CMAKE_COMMAND} -E touch ${CMAKE_BINARY_DIR}/cstyle-${name}-status
			)
	endif()

	add_custom_target(cstyle-${name}
			DEPENDS ${CMAKE_BINARY_DIR}/cstyle-${name}-status)
	add_dependencies(cstyle cstyle-${name})
endfunction()

# Generates check-whitespace-$name target and attaches it as a dependency
# of global "check-whitespace" target.
# ${name} must be unique.
function(add_check_whitespace name)
	if(${ARGC} EQUAL 1)
		add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/check-whitespace-${name}-status
			DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/*.c
				${CMAKE_CURRENT_SOURCE_DIR}/*.h
			COMMAND ${PERL_EXECUTABLE}
				${CMAKE_SOURCE_DIR}/utils/check_whitespace
				${CMAKE_CURRENT_SOURCE_DIR}/*.c
				${CMAKE_CURRENT_SOURCE_DIR}/*.h
			COMMAND ${CMAKE_COMMAND} -E touch ${CMAKE_BINARY_DIR}/check-whitespace-${name}-status)
	else()
		add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/check-whitespace-${name}-status
			DEPENDS ${ARGN}
			COMMAND ${PERL_EXECUTABLE}
				${CMAKE_SOURCE_DIR}/utils/check_whitespace ${ARGN}
			COMMAND ${CMAKE_COMMAND} -E touch ${CMAKE_BINARY_DIR}/check-whitespace-${name}-status)
	endif()

	add_custom_target(check-whitespace-${name}
			DEPENDS ${CMAKE_BINARY_DIR}/check-whitespace-${name}-status)
	add_dependencies(check-whitespace check-whitespace-${name})
endfunction()

# Sets ${ret} to version of program specified by ${name} in major.minor format
function(get_program_version_major_minor name ret)
	execute_process(COMMAND ${name} --version
		OUTPUT_VARIABLE cmd_ret
		ERROR_QUIET)
	STRING(REGEX MATCH "([0-9]+.)([0-9]+)" VERSION ${cmd_ret})
	SET(${ret} ${VERSION} PARENT_SCOPE)
endfunction()

function(find_pmemcheck)
	set(ENV{PATH} ${VALGRIND_PREFIX}/bin:$ENV{PATH})
	execute_process(COMMAND valgrind --tool=pmemcheck --help
			RESULT_VARIABLE VALGRIND_PMEMCHECK_RET
			OUTPUT_QUIET
			ERROR_QUIET)
	if(VALGRIND_PMEMCHECK_RET)
		set(VALGRIND_PMEMCHECK_FOUND 0 CACHE INTERNAL "")
	else()
		set(VALGRIND_PMEMCHECK_FOUND 1 CACHE INTERNAL "")
	endif()

	if(VALGRIND_PMEMCHECK_FOUND)
		execute_process(COMMAND valgrind --tool=pmemcheck true
				ERROR_VARIABLE PMEMCHECK_OUT
				OUTPUT_QUIET)

		string(REGEX MATCH ".*pmemcheck-([0-9.]*),.*" PMEMCHECK_OUT "${PMEMCHECK_OUT}")
		set(PMEMCHECK_VERSION ${CMAKE_MATCH_1} CACHE INTERNAL "")
	else()
		message(WARNING "Valgrind pmemcheck NOT found. Pmemcheck tests will not be performed.")
	endif()
endfunction()

# check if libibverbs has ODP support
function(is_ODP_supported var)
	CHECK_C_SOURCE_COMPILES("
		#include <infiniband/verbs.h>
		/* check if 'IBV_ACCESS_ON_DEMAND is defined */
		int main() {
			if (!IBV_ACCESS_ON_DEMAND)
				return -1;
			return 0;
		}"
		ON_DEMAND_PAGING_SUPPORTED)
	set(var ${ON_DEMAND_PAGING_SUPPORTED} PARENT_SCOPE)
endfunction()

# check if librdmacm has correct signature of rdma_getaddrinfo()
function(check_signature_rdma_getaddrinfo var)
	if(${CMAKE_C_COMPILER} MATCHES "gcc")
		# check if the GCC compiler supports the '-Werror=discarded-qualifiers' flag
		CHECK_C_COMPILER_FLAG("-Werror=discarded-qualifiers" C_HAS_Werror_discarded_qualifiers)
		if(C_HAS_Werror_discarded_qualifiers)
			set(RUN_CHECK_C_SOURCE_COMPILES 1)
		endif()
	else()
		# the clang compiler ignores the '-Werror=discarded-qualifiers' flag
		set(RUN_CHECK_C_SOURCE_COMPILES 1)
	endif()

	if(RUN_CHECK_C_SOURCE_COMPILES)
		set(CMAKE_REQUIRED_FLAGS "-Werror=discarded-qualifiers;${CMAKE_REQUIRED_FLAGS}")
		set(CMAKE_REQUIRED_LIBRARIES "-lrdmacm;${CMAKE_REQUIRED_LIBRARIES}")

		CHECK_C_SOURCE_COMPILES("
			#include <rdma/rdma_cma.h>
			int main() {
				const char *node;
				const char *service;
				const struct rdma_addrinfo *hints;
				struct rdma_addrinfo **res;
				if (rdma_getaddrinfo(node, service, hints, res))
					return -1;
				return 0;
			}"
			SIGNATURE_OK_RDMA_GETADDRINFO)
		set(var ${SIGNATURE_OK_RDMA_GETADDRINFO} PARENT_SCOPE)
		return()
	endif()

	#
	# We are running an old version of the GCC compiler
	# that does not support the '-Werror=discarded-qualifiers' flag.
	#
	message(NOTICE "-- Performing Test SIGNATURE_OK_RDMA_GETADDRINFO")
	find_file(RDMA_CMA_H rdma_cma.h PATHS /usr/include/rdma /usr/include)
	if(NOT RDMA_CMA_H)
		message(FATAL_ERROR "Cannot find the 'rdma_cma.h' header file!")
	endif()
	file(STRINGS ${RDMA_CMA_H} CORRECT_SIGNATURE_FOUND REGEX "int rdma_getaddrinfo[(]const char")
	if(CORRECT_SIGNATURE_FOUND)
		message(NOTICE "-- Performing Test SIGNATURE_OK_RDMA_GETADDRINFO - Success")
		# XXX It should be:
		#    set(var 1 PARENT_SCOPE)
		# but for an unknown reason it does not work.
		set(SIGNATURE_OK_RDMA_GETADDRINFO 1 PARENT_SCOPE)
	else()
		message(NOTICE "-- Performing Test SIGNATURE_OK_RDMA_GETADDRINFO - Failed")
	endif()
endfunction()
