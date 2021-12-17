#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021, Intel Corporation
#

#
# runner.py
#

"""the abstract runner (EXPERIMENTAL)"""

class Runner:
    """the abstract runner

    The abstract runner covers common runners functionality.
    """

    def __init__(self, benchmark, config: dict, idfile: str) -> 'Runner':
        # XXX nice to have REMOTE_JOB_NUMA_CPULIST, CORES_PER_SOCKET
        self.__benchmark = benchmark
        self.__config = config
        self.__idfile = idfile
        self.__server = None
