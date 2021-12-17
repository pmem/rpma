#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021, Intel Corporation
#

#
# __init__.py -- required for python imports
#

"""Controlling generating the final report

To initiate the benchmarking process you provide figures in sets known as parts
(`report_bench`, `lib.bench.Bench.parts`). This package combines these
parts and figures you have already generated into a final
`lib.report.base.Report`. For details please see `lib`.
"""

from .base import Report
