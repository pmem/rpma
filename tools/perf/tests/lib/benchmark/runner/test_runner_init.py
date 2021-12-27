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
    """test proper initialization of Runner object"""
 
    oneseries = {'tool': 'tool', 'mode': 'mode', 'tool_mode': 'tool_mode'}
    benchmark = lib.benchmark.Benchmark(oneseries)
    config = {'server_ip':'server_ip'}
    
    runner = Runner(benchmark, config, 'idfile')
    assert runner._benchmark == benchmark
    assert runner._config == config
    assert runner._idfile == 'idfile'
    
    assert runner._tool == oneseries['tool']
    assert runner._tool_mode == oneseries['tool_mode']
    assert runner._mode == oneseries['mode']
    assert runner._config['server_ip'] == 'server_ip'
    assert runner._benchmark.oneseries['tool'] == 'tool'

def test_Runner_init_failed_config_no_server_ip():
    """test failed initialization of Runner object
        - no server_ip in config"""
 
    oneseries = {'tool': 'tool', 'mode': 'mode', 'tool_mode': 'tool_mode'}
    benchmark = lib.benchmark.Benchmark(oneseries)
    config = {}
    with pytest.raises(ValueError):
        runner = Runner(benchmark, config, 'idfile')

def test_Runner_init_failed_no_config():
    """test failed initialization of Runner object
        - no config"""
 
    oneseries = {'tool': 'tool', 'mode': 'mode', 'tool_mode': 'tool_mode'}
    benchmark = lib.benchmark.Benchmark(oneseries)
    with pytest.raises(RuntimeError):
        runner = Runner(benchmark, None, 'idfile')
