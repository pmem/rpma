#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021-2022, Intel Corporation
#

add_cstyle_pep8(tools_perf
	# XXX requires adding pytest to all CI images
	# ${CMAKE_CURRENT_SOURCE_DIR}/tests/*.py
	# ${CMAKE_CURRENT_SOURCE_DIR}/tests/*/*.py
	# ${CMAKE_CURRENT_SOURCE_DIR}/tests/*/*/*.py
	# ${CMAKE_CURRENT_SOURCE_DIR}/tests/*/*/*/*.py
	${CMAKE_CURRENT_SOURCE_DIR}/report_bench.py
	${CMAKE_CURRENT_SOURCE_DIR}/report_create.py 
	${CMAKE_CURRENT_SOURCE_DIR}/report_figures.py
	${CMAKE_CURRENT_SOURCE_DIR}/lib/__init__.py
	${CMAKE_CURRENT_SOURCE_DIR}/lib/bench.py
	${CMAKE_CURRENT_SOURCE_DIR}/lib/common.py
	${CMAKE_CURRENT_SOURCE_DIR}/lib/compare.py
	${CMAKE_CURRENT_SOURCE_DIR}/lib/remote_cmd.py
	${CMAKE_CURRENT_SOURCE_DIR}/lib/figure/*.py
	${CMAKE_CURRENT_SOURCE_DIR}/lib/format/*.py
	${CMAKE_CURRENT_SOURCE_DIR}/lib/benchmark/*.py
	${CMAKE_CURRENT_SOURCE_DIR}/lib/benchmark/runner/*.py
	${CMAKE_CURRENT_SOURCE_DIR}/lib/report/*.py
	${CMAKE_CURRENT_SOURCE_DIR}/tests/lib/benchmark/*.py
	${CMAKE_CURRENT_SOURCE_DIR}/tests/lib/benchmark/runner/*.py)
