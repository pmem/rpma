#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021, Intel Corporation
#

#
# runner.py
#

"""the abstract runner (EXPERIMENTAL)"""

from .common import UNKNOWN_VALUE_MSG, NO_X_AXIS_MSG, MISSING_KEY_MSG, \
                    BS_VALUES, run_pre_command, run_post_command, \
                    result_append, result_is_done, print_start_message

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

    def __validate(self):
        for key in self.__ONESERIES_REQUIRED:
            if key not in self._benchmark.oneseries:
                raise ValueError(MISSING_KEY_MSG.format(key))

        if self._config == None:
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
    def _mode(self):
        return self._benchmark.oneseries['mode']

    @property
    def _tool_mode(self):
        return self._benchmark.oneseries['tool_mode']

    @property
    def _tool(self):
        return self._benchmark.oneseries['tool']

    __ONESERIES_REQUIRED = ['tool', 'mode']
    __CONFIG_REQUIRED = ['server_ip']
