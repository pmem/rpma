#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2018-2022, Intel Corporation
#

#
# ctest_helpers.cmake - helper functions for tests/CMakeLists.txt
#

set(TEST_ROOT_DIR ${PROJECT_SOURCE_DIR}/tests)
set(TEST_UNIT_COMMON_DIR ${TEST_ROOT_DIR}/unit/common)
set(TEST_MT_COMMON_DIR ${TEST_ROOT_DIR}/multithreaded/common)

set(GLOBAL_TEST_ARGS
	-DPERL_EXECUTABLE=${PERL_EXECUTABLE}
	-DMATCH_SCRIPT=${PROJECT_SOURCE_DIR}/tests/match
	-DPARENT_DIR=${TEST_DIR}
	-DTESTS_USE_FORCED_PMEM=${TESTS_USE_FORCED_PMEM}
	-DTEST_ROOT_DIR=${TEST_ROOT_DIR})

if(TESTS_VERBOSE_OUTPUT)
	set(GLOBAL_TEST_ARGS ${GLOBAL_TEST_ARGS} --trace-expand)
endif()

set(INCLUDE_DIRS .. .)

include_directories(${INCLUDE_DIRS})
link_directories(${LIBS_DIRS})

function(find_gdb)
	execute_process(COMMAND gdb --help
			RESULT_VARIABLE GDB_RET
			OUTPUT_QUIET
			ERROR_QUIET)
	if(GDB_RET)
		set(GDB_FOUND 0 CACHE INTERNAL "")
		message(WARNING "GDB NOT found, some tests will be skipped")
	else()
		set(GDB_FOUND 1 CACHE INTERNAL "")
	endif()
endfunction()

function(find_packages)
	pkg_check_modules(CMOCKA REQUIRED cmocka)

	if(NOT CMOCKA_FOUND)
		message(FATAL_ERROR "Cmocka not found. Cmocka is required to run tests.")
	endif()
endfunction()

# Function to build test with custom build options (e.g. passing defines)
# Example: build_test_ext(NAME ... SRC_FILES ....c BUILD_OPTIONS -D...)
function(build_test_lib_ext)
	set(oneValueArgs NAME)
	set(multiValueArgs SRC_FILES BUILD_OPTIONS)
	cmake_parse_arguments(TEST "" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

	build_test_lib(${TEST_NAME} ${TEST_SRC_FILES})
	target_compile_definitions(${TEST_NAME} PRIVATE ${TEST_BUILD_OPTIONS})
endfunction()

function(build_test_lib name)
	set(srcs ${ARGN})
	prepend(srcs ${CMAKE_CURRENT_SOURCE_DIR} ${srcs})

	add_executable(${name} ${srcs})
	target_link_libraries(${name} rpma cmocka)
	target_include_directories(${name} PRIVATE ${LIBRPMA_INCLUDE_DIRS})

	add_dependencies(tests ${name})
endfunction()

function(build_test_src)
	set(options UNIT)
	set(oneValueArgs NAME)
	set(multiValueArgs SRCS)
	cmake_parse_arguments(TEST
		"${options}"
		"${oneValueArgs}"
		"${multiValueArgs}"
		${ARGN})

	add_executable(${TEST_NAME} ${TEST_SRCS})
	target_include_directories(${TEST_NAME} PRIVATE
		${LIBRPMA_INCLUDE_DIRS}
		${LIBRPMA_SOURCE_DIR})
	if(TEST_UNIT)
		target_include_directories(${TEST_NAME} PRIVATE
			${TEST_UNIT_COMMON_DIR})
	endif()
	# do not link with the rpma library
	target_link_libraries(${TEST_NAME} cmocka)

	add_dependencies(tests ${TEST_NAME})
endfunction()

set(vg_tracers memcheck helgrind drd pmemcheck)

# Configures testcase ${name} ${testcase} using tracer ${tracer}, cmake_script is used to run test
function(add_testcase name tracer testcase cmake_script)
	set(executable ${name})
	add_test(NAME ${executable}_${testcase}_${tracer}
			COMMAND ${CMAKE_COMMAND}
			${GLOBAL_TEST_ARGS}
			-DTEST_NAME=${executable}_${testcase}_${tracer}
			-DTESTCASE=${testcase}
			-DSRC_DIR=${CMAKE_CURRENT_SOURCE_DIR}
			-DBIN_DIR=${CMAKE_CURRENT_BINARY_DIR}/${executable}_${testcase}_${tracer}
			-DTEST_EXECUTABLE=$<TARGET_FILE:${executable}>
			-DTRACER=${tracer}
			-DLONG_TESTS=${LONG_TESTS}
			-DMAX_THREADS=${MAX_THREADS}
			-DVALGRIND_S_OPTION=${VALGRIND_S_OPTION}
			-P ${cmake_script})

	set_tests_properties(${name}_${testcase}_${tracer} PROPERTIES
			ENVIRONMENT "LC_ALL=C;PATH=$ENV{PATH};"
			FAIL_REGULAR_EXPRESSION Sanitizer)

	if (${tracer} STREQUAL pmemcheck)
		# XXX: if we use FATAL_ERROR in test.cmake - pmemcheck passes anyway
		set_tests_properties(${name}_${testcase}_${tracer} PROPERTIES
				FAIL_REGULAR_EXPRESSION "CMake Error")
	endif()
endfunction()

function(skip_test name message)
	add_test(NAME ${name}_${message}
		COMMAND ${CMAKE_COMMAND} -P ${TEST_ROOT_DIR}/true.cmake)

	set_tests_properties(${name}_${message} PROPERTIES COST 0)
endfunction()

# adds testcase with name, tracer, and cmake_script responsible for running it
function(add_test_common name tracer testcase cmake_script)
	if(${tracer} STREQUAL "")
	    set(tracer none)
	endif()

	if (${tracer} IN_LIST vg_tracers)
		if (NOT VALGRIND_FOUND)
			skip_test(${name}_${testcase}_${tracer} "SKIPPED_BECAUSE_OF_MISSING_VALGRIND")
			return()
		endif()
		if (DEBUG_USE_ASAN OR DEBUG_USE_UBSAN)
			skip_test(${name}_${testcase}_${tracer} "SKIPPED_BECAUSE_SANITIZER_USED")
			return()
		endif()
	endif()

	if (${tracer} STREQUAL "pmemcheck" AND NOT VALGRIND_PMEMCHECK_FOUND)
		skip_test(${name}_${testcase}_${tracer} "SKIPPED_BECAUSE_OF_MISSING_PMEMCHECK")
		return()
	endif()

	# if test was not build
	if (NOT TARGET ${name})
		return()
	endif()

	add_testcase(${name} ${tracer} ${testcase} ${cmake_script})
endfunction()

function(add_test_generic)
	set(options GROUP_SCRIPT)
	set(oneValueArgs NAME SCRIPT CASE)
	set(multiValueArgs TRACERS)
	cmake_parse_arguments(TEST "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

	if("${TEST_CASE}" STREQUAL "")
		set(STR_TEST_CASE "")
	else()
		set(STR_TEST_CASE "_${TEST_CASE}")
	endif()

	if(NOT "${TEST_SCRIPT}" STREQUAL "")
		# SCRIPT is set
		set(cmake_script ${CMAKE_CURRENT_SOURCE_DIR}/${TEST_SCRIPT})
	elseif(TEST_GROUP_SCRIPT)
		# GROUP_SCRIPT is set
		set(cmake_script ${CMAKE_CURRENT_SOURCE_DIR}/../run_group${STR_TEST_CASE}.cmake)
	elseif(NOT "${TEST_CASE}" STREQUAL "")
		# CASE is set
		set(cmake_script ${CMAKE_CURRENT_SOURCE_DIR}/${TEST_NAME}${STR_TEST_CASE}.cmake)
	else()
		# none of: SCRIPT nor GROUP_SCRIPT nor CASE is set
		set(cmake_script ${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/run_default.cmake)
	endif()

	if("${TEST_CASE}" STREQUAL "")
		set(TEST_CASE "0") # TEST_CASE is required by add_test_common()
	endif()

	if("${TEST_TRACERS}" STREQUAL "")
		add_test_common(${TEST_NAME} none ${TEST_CASE} ${cmake_script})
	else()
		foreach(tracer ${TEST_TRACERS})
			add_test_common(${TEST_NAME} ${tracer} ${TEST_CASE} ${cmake_script})
		endforeach()
	endif()
endfunction()

function(add_multithreaded)
	set(options USE_LIBIBVERBS)
	set(oneValueArgs NAME BIN)
	set(multiValueArgs SRCS)
	cmake_parse_arguments(MULTITHREADED
		"${options}"
		"${oneValueArgs}"
		"${multiValueArgs}"
		${ARGN})

	set(target mtt-${MULTITHREADED_NAME}-${MULTITHREADED_BIN})

	prepend(srcs ${CMAKE_CURRENT_SOURCE_DIR} ${srcs})
	add_executable(${target} ${TEST_MT_COMMON_DIR}/mtt.c
		${MULTITHREADED_SRCS})
	target_include_directories(${target} PRIVATE
		${TEST_MT_COMMON_DIR} ${LIBRPMA_INCLUDE_DIRS})
	set_target_properties(${target} PROPERTIES
		OUTPUT_NAME ${MULTITHREADED_BIN})
	target_link_libraries(${target} ${LIBRPMA_LIBRARIES} pthread)

	if(MULTITHREADED_USE_LIBIBVERBS)
		target_include_directories(${target}
			PRIVATE ${LIBIBVERBS_INCLUDE_DIRS})
		target_link_libraries(${target} ${LIBIBVERBS_LIBRARIES})
	endif()

	add_test_generic(NAME ${target} GROUP_SCRIPT TRACERS none memcheck drd helgrind)
endfunction()
