#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021, Intel Corporation
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
	${CMAKE_CURRENT_SOURCE_DIR}/lib/bench.py
	${CMAKE_CURRENT_SOURCE_DIR}/lib/figure.py)
