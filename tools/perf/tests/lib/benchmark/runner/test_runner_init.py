#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021, Intel Corporation
#

"""test_runner_init.py -- lib.benchmark.runner.Runner init tests"""

import pytest

import lib.benchmark
import lib.benchmark.runner
import lib.benchmark.runner.runner
from lib.benchmark.base import Benchmark

from lib.benchmark.runner.runner import Runner
__ONESERIES_MINI = {'tool': 'tool', 'mode': 'mode', 'tool_mode': 'tool_mode'}
testdata = [
    (True),
    (False),
]
@pytest.mark.parametrize("expected_dump_cmds", testdata)
def test_Runner_init(expected_dump_cmds):
    """test proper initialization of Runner object"""

    benchmark = lib.benchmark.Benchmark(__ONESERIES_MINI)
    config = {'server_ip':'server_ip', 'DUMP_CMDS': expected_dump_cmds}

    runner = Runner(benchmark, config, 'idfile')
    assert runner._benchmark == benchmark
    assert runner._config == config
    assert runner._idfile == 'idfile'
    assert runner._dump_cmds == expected_dump_cmds

    assert runner._tool == __ONESERIES_MINI['tool']
    assert runner._tool_mode == __ONESERIES_MINI['tool_mode']
    assert runner._mode == __ONESERIES_MINI['mode']
    assert runner._config['server_ip'] == 'server_ip'
    assert runner._benchmark.__ONESERIES_MINI['tool'] == 'tool'

def test_Runner_init_failed_config_no_server_ip():
    """test failed initialization of Runner object
        - no server_ip in config"""

    benchmark = lib.benchmark.Benchmark(__ONESERIES_MINI)
    config = {}
    with pytest.raises(ValueError):
        runner = Runner(benchmark, config, 'idfile')

def test_Runner_init_failed_config_no_server_ip():
    """test failed initialization of Runner object
        - no server_ip in config"""

    benchmark = lib.benchmark.Benchmark(__ONESERIES_MINI)
    config = {}
    with pytest.raises(ValueError):
        runner = Runner(benchmark, config, 'idfile')

def test_Runner_init_failed_no_config():
    """test failed initialization of Runner object
        - no config"""

    benchmark = lib.benchmark.Benchmark(__ONESERIES_MINI)
    with pytest.raises(RuntimeError):
        runner = Runner(benchmark, None, 'idfile')
