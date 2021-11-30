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
benchmarking conditions when a made requirements are met.
"""

from .base import Benchmark, get_result_path
