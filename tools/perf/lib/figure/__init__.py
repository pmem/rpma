#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021, Intel Corporation
#

#
# __init__.py -- required for python imports
#

"""Controlling the benchmarking process related to a single figure

A figure is a set of data series accompanied by the figure's metadata.
Each of the series is a list of data points that translates to a single
`lib.benchmark.base.Benchmark` instance.

**Note**: The definition of a figure on the input may be multidimensional.
`lib.figure.flat.flatten` is the default way to create a list of
`lib.figure.base.Figure` instances from the part's figure JSON input file.
For more details about the general flow of processing please see `lib`.
"""

from .base import Figure
from .flat import flatten
from .image import draw_png
