#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021, Intel Corporation
#

add_cstyle_pep8(tools_perf
	${CMAKE_CURRENT_SOURCE_DIR}/report_bench.py
	${CMAKE_CURRENT_SOURCE_DIR}/report_create.py 
	${CMAKE_CURRENT_SOURCE_DIR}/report_figures.py)
