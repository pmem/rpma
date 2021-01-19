#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2018-2021, Intel Corporation
#

#
# ctest_helpers.cmake - helper functions for tests/CMakeLists.txt
#

set(TEST_ROOT_DIR ${PROJECT_SOURCE_DIR}/tests)
set(TEST_UNIT_COMMON_DIR ${TEST_ROOT_DIR}/unit/common)

set(GLOBAL_TEST_ARGS
	-DPERL_EXECUTABLE=${PERL_EXECUTABLE}
	-DMATCH_SCRIPT=${PROJECT_SOURCE_DIR}/tests/match
	-DPARENT_DIR=${TEST_DIR}
	-DTESTS_USE_FORCED_PMEM=${TESTS_USE_FORCED_PMEM}
	-DTEST_ROOT_DIR=${TEST_ROOT_DIR})

if(TRACE_TESTS)
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
	pkg_check_modules(LIBUNWIND QUIET libunwind)

	if(NOT CMOCKA_FOUND)
		message(FATAL_ERROR "Cmocka not found. Cmocka is required to run tests.")
	endif()

	if(NOT LIBUNWIND_FOUND)
		message(WARNING "libunwind-dev/devel not found. Stack traces from tests will not be reliable")
	endif()

	if(TESTS_USE_VALGRIND AND NOT VALGRIND_FOUND)
		message(WARNING "Valgrind not found. Valgrind tests will not be performed.")
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
	target_link_libraries(${name} rpma cmocka test_backtrace)
	target_include_directories(${name} PRIVATE ${LIBRPMA_INCLUDE_DIRS})
	if(LIBUNWIND_FOUND)
		target_link_libraries(${name} ${LIBUNWIND_LIBRARIES} ${CMAKE_DL_LIBS})
	endif()

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
	target_link_libraries(${TEST_NAME} cmocka test_backtrace)
	if(LIBUNWIND_FOUND)
		target_link_libraries(${TEST_NAME} ${LIBUNWIND_LIBRARIES} ${CMAKE_DL_LIBS})
	endif()

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
			-P ${cmake_script})

	set_tests_properties(${name}_${testcase}_${tracer} PROPERTIES
			ENVIRONMENT "LC_ALL=C;PATH=$ENV{PATH};"
			FAIL_REGULAR_EXPRESSION Sanitizer)

	if (${tracer} STREQUAL pmemcheck)
		# XXX: if we use FATAL_ERROR in test.cmake - pmemcheck passes anyway
		set_tests_properties(${name}_${testcase}_${tracer} PROPERTIES
				FAIL_REGULAR_EXPRESSION "CMake Error")
	endif()

	if (${tracer} STREQUAL pmemcheck)
		set_tests_properties(${name}_${testcase}_${tracer} PROPERTIES
				COST 100)
	elseif(${tracer} IN_LIST vg_tracers)
		set_tests_properties(${name}_${testcase}_${tracer} PROPERTIES
				COST 50)
	else()
		set_tests_properties(${name}_${testcase}_${tracer} PROPERTIES
				COST 10)
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

	if (((NOT VALGRIND_FOUND) OR (NOT TESTS_USE_VALGRIND)) AND ${tracer} IN_LIST vg_tracers)
		# Only print "SKIPPED_*" message when option is enabled
		if (TESTS_USE_VALGRIND)
			skip_test(${name}_${testcase}_${tracer} "SKIPPED_BECAUSE_OF_MISSING_VALGRIND")
		endif()
		return()
	endif()

	if (((NOT VALGRIND_PMEMCHECK_FOUND) OR (NOT TESTS_USE_VALGRIND)) AND ${tracer} STREQUAL "pmemcheck")
		# Only print "SKIPPED_*" message when option is enabled
		if (TESTS_USE_VALGRIND)
			skip_test(${name}_${testcase}_${tracer} "SKIPPED_BECAUSE_OF_MISSING_PMEMCHECK")
		endif()
		return()
	endif()

	if ((USE_ASAN OR USE_UBSAN) AND ${tracer} IN_LIST vg_tracers)
		skip_test(${name}_${testcase}_${tracer} "SKIPPED_BECAUSE_SANITIZER_USED")
		return()
	endif()

	# if test was not build
	if (NOT TARGET ${name})
		return()
	endif()

	add_testcase(${name} ${tracer} ${testcase} ${cmake_script})
endfunction()

function(add_test_generic)
	set(oneValueArgs NAME CASE SCRIPT)
	set(multiValueArgs TRACERS)
	cmake_parse_arguments(TEST "" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

	if("${TEST_SCRIPT}" STREQUAL "")
		if("${TEST_CASE}" STREQUAL "")
			set(TEST_CASE "0")
			set(cmake_script ${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/run_default.cmake)
		else()
			set(cmake_script ${CMAKE_CURRENT_SOURCE_DIR}/${TEST_NAME}_${TEST_CASE}.cmake)
		endif()
	else()
		if("${TEST_CASE}" STREQUAL "")
			set(TEST_CASE "0")
		endif()
		set(cmake_script ${CMAKE_CURRENT_SOURCE_DIR}/${TEST_SCRIPT})
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

	set(target multithreaded-${MULTITHREADED_NAME}-${MULTITHREADED_BIN})

	prepend(srcs ${CMAKE_CURRENT_SOURCE_DIR} ${srcs})
	add_executable(${target} ${MULTITHREADED_SRCS})
	target_include_directories(${target} PRIVATE
		${LIBRPMA_INCLUDE_DIRS})
	set_target_properties(${target} PROPERTIES
		OUTPUT_NAME ${MULTITHREADED_BIN})
	target_link_libraries(${target} ${LIBRPMA_LIBRARIES} pthread)

	if(MULTITHREADED_USE_LIBIBVERBS)
		target_include_directories(${target}
			PRIVATE ${LIBIBVERBS_INCLUDE_DIRS})
		target_link_libraries(${target} ${LIBIBVERBS_LIBRARIES})
	endif()

	add_test_generic(NAME ${target} CASE 0)
endfunction()
