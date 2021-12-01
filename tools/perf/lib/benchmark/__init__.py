#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021, Intel Corporation
#

#
# __init__.py -- required for python imports
#

"""Controlling the benchmarking process related to a single series

A single series is a list of data points collected under well defined
benchmarking conditions when the given requirements are met.
A `lib.benchmark.base.Benchmark` is an entity working towards collecting
these data points. So all the `lib.figure.base.Figure` instances (which have
ordered the given series) can collect their results.
"""

from .base import Benchmark, get_result_path
