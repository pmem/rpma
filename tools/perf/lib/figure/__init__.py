#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021, Intel Corporation
#

#
# __init__.py -- required for python imports
#

"""Controlling a benchmarking process related to a single figure

A figure is a set of data series accompanied by the figure metadata.
Each of the series is a list of data points that translates to a single
`lib.benchmark.base.Benchmark` instance.

Knowing that figures definition on the input may be multidimensional,
the default way to create a list of `lib.figure.base.Figure` instances covering
a given part's figure JSON input file is to use `lib.figure.flat.flatten`.
For more details about the general flow of processing please see `lib`.
"""

from .base import Figure
from .flat import flatten
