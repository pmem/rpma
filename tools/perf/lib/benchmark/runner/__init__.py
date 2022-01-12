#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021-2022, Intel Corporation
#

#
# __init__.py -- required for python imports
#

"""Executing the benchmarking process related to a single series

Taking into account many circumstances `lib.benchmark.base.Benchmark` may
choose to collect the results using one of available runners
(Bash runner is DEPRECATED).
Please see `lib.benchmark.base.Benchmark.run()` for details.
"""

from .base import BaseRunner
from .bash import Bash
from .dummy import Dummy
