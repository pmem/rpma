#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021, Intel Corporation
#

"""test_runner.py -- lib.benchmark.runner.Runner tests"""

import pytest

import lib.benchmark
import lib.benchmark.runner
import lib.benchmark.runner.runner
from lib.benchmark.base import Benchmark

from lib.benchmark.runner.runner import Runner

def test_Runner_init():
    """test proper initializationof Runner object"""
 
    oneseries = {'tool': 'tool', 'mode': 'mode', 'tool_mode': 'tool_mode'}
    config = {'server_ip':'server_ip'}
    
    benchmark = lib.benchmark.Benchmark(oneseries)
    runner = Runner(benchmark, config, 'idfile')
    assert runner._benchmark == benchmark
    assert runner._config == config
    assert runner._idfile == 'idfile'
    
    assert runner._tool == oneseries['tool']
    assert runner._tool_mode == oneseries['tool_mode']
    assert runner._mode == oneseries['mode']
    assert runner._config['server_ip'] == 'server_ip'
    assert runner._benchmark.oneseries['tool'] == 'tool'
    