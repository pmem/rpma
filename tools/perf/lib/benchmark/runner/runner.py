#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021, Intel Corporation
#

#
# runner.py
#

"""the abstract runner (EXPERIMENTAL)"""
#from ..base import Benchmark

from .common import MISSING_KEY_MSG

class Runner:
    """the abstract runner

    The abstract runner covers common runners functionality.
    """

    def __init__(self, benchmark, config: dict, idfile: str) -> 'Runner':
        # XXX nice to have REMOTE_JOB_NUMA_CPULIST, CORES_PER_SOCKET
        self.__benchmark = benchmark
        self.__config = config
        self.__idfile = idfile

    @property
    def _benchmark(self):
        return self.__benchmark

    @property
    def _config(self):
        return self.__config

    @property
    def _idfile(self):
        return self.__idfile

    @property
    def _oneseries(self):
        return self._benchmark.oneseries
