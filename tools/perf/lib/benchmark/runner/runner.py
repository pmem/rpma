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
        self.__validate()
        # set dumping commands
        self.__dump_cmds = self._config.get('DUMP_CMDS', False)


    def __validate(self):
        if self._benchmark is None:
            raise RuntimeError("Benchmark is missing")

        if self._oneseries is None:
            raise RuntimeError("OneSeries is missing")

        for key in self.__ONESERIES_REQUIRED:
            if key not in self._benchmark.oneseries:
                raise ValueError(MISSING_KEY_MSG.format(key))

        if self._config is None:
            raise RuntimeError("Config is missing")

        for key in self.__CONFIG_REQUIRED:
            if key not in self._config:
                raise ValueError(MISSING_KEY_MSG.format(key))

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

    @property
    def _dump_cmds(self):
        return self.__dump_cmds

    @property
    def _tool(self):
        return self._oneseries['tool']

    @property
    def _tool_mode(self):
        return self._oneseries['tool_mode']

    @property
    def _mode(self):
        return self._oneseries['mode']

    __ONESERIES_REQUIRED = ['tool', 'tool_mode', 'mode']
    __CONFIG_REQUIRED = ['server_ip']
