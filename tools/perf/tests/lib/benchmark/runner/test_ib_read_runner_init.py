#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021, Intel Corporation
#

"""test_ib_read_runner_init.py
   -- lib.benchmark.runner.ib_read.IBReadRunner init tests"""

import shutil
import pytest

import lib.benchmark
import lib.benchmark.runner
import lib.benchmark.runner.ib_read

from lib.remote_cmd import RemoteCmd
from lib.benchmark.runner.ib_read import IbReadRunner

CONFIG_MINI = {'server_ip':'server_ip'}

def test_ib_read_runner_init(monkeypatch):
    """test proper initialization of IbReadRunner object
       with all mandatory parameters"""

    def which_mock(path: str) -> str:
        assert path == 'ib_read_lat'
        return path

    def run_sync_mock(_arg1, _arg2) -> RemoteCmd:
        return RemoteCmd(None, None, None, exit_status=0)

    def run_mock(_self) -> None:
        pass

    monkeypatch.setattr(shutil, 'which', which_mock)
    monkeypatch.setattr(RemoteCmd, 'run_sync', run_sync_mock)
    monkeypatch.setattr(IbReadRunner, 'run', run_mock)

    oneseries = {'tool': 'ib_read', 'tool_mode': 'lat', 'mode': 'lat',
                 'rw': 'read', 'filetype': 'malloc',
                 'requirements': {'direct_write_to_pmem': True}}

    benchmark = lib.benchmark.Benchmark(oneseries)
    runner = None

    runner = IbReadRunner(benchmark, CONFIG_MINI, 'idfile')
    runner.run()

    #pylint: disable=protected-access
    assert runner._benchmark == benchmark
    assert runner._config == CONFIG_MINI
    assert runner._idfile == 'idfile'
    assert runner._tool == oneseries['tool']
    assert runner._tool_mode == oneseries['tool_mode']
    assert runner._mode == oneseries['mode']
    assert runner._config['server_ip'] == CONFIG_MINI['server_ip']
    #pylint: enable=protected-access

def test_ib_read_runner_init_oneserises_no_tool():
    """failed initialization of IbReadRunner object - no tool param provided """
    oneseries = {}
    benchmark = lib.benchmark.Benchmark(oneseries)
    runner = None

    with pytest.raises(ValueError):
        runner = IbReadRunner(benchmark, CONFIG_MINI, 'idfile')

    assert runner is None

def test_ib_read_runner_init_oneserises_no_toolmode():
    """failed initialization of IbReadRunner object - no tool_mode param provided """
    oneseries = {'tool': 'ib_read'}
    benchmark = lib.benchmark.Benchmark(oneseries)
    runner = None

    with pytest.raises(ValueError):
        runner = IbReadRunner(benchmark, CONFIG_MINI, 'idfile')

    assert runner is None

def test_ib_read_runner_init_oneserises_no_mode():
    """failed initialization of IbReadRunner object - no mode param provided """
    oneseries = {'tool': 'ib_read', 'tool_mode': 'lat'}
    benchmark = lib.benchmark.Benchmark(oneseries)
    runner = None

    with pytest.raises(ValueError):
        runner = IbReadRunner(benchmark, CONFIG_MINI, 'idfile')

    assert runner is None

def test_ib_read_runner_init_oneserises_no_rw():
    """failed initialization of IbReadRunner object - no mode rw provided """
    oneseries = {'tool': 'ib_read', 'mode': 'lat'}
    benchmark = lib.benchmark.Benchmark(oneseries)
    runner = None

    with pytest.raises(ValueError):
        IbReadRunner(benchmark, CONFIG_MINI, 'idfile')

def test_ib_read_runner_init_oneserises_no_filetype():
    """failed initialization of IbReadRunner object - no mode rw provided """
    oneseries = {'tool': 'ib_read', 'mode': 'lat', 'rw':'read'}
    benchmark = lib.benchmark.Benchmark(oneseries)
    runner = None

    with pytest.raises(ValueError):
        runner = IbReadRunner(benchmark, CONFIG_MINI, 'idfile')
    assert runner is None

def test_ib_read_runner_init_no_config():
    """failed initialization of IbReadRunner object - no config provided """
    oneseries = {'tool': 'ib_read', 'tool_mode': 'lat', 'mode': 'lat',
                 'rw':'read', 'filetype': 'malloc',
                 'requirements': {}}
    benchmark = lib.benchmark.Benchmark(oneseries)
    runner = None

    with pytest.raises(RuntimeError):
        runner = IbReadRunner(benchmark, None, 'idfile')

    assert runner is None

def test_ib_read_runner_init_config_no_server_ip(monkeypatch):
    """failed initialization of IbReadRunner object -
       - no server_ip in config provided """
    def which_mock(path: str) ->str:
        assert path == 'ib_read_lat'
        return path
    monkeypatch.setattr(shutil, 'which', which_mock)

    oneseries = {'tool': 'ib_read', 'mode': 'lat',
                 'rw':'read', 'filetype': 'malloc'}
    config = {}
    benchmark = lib.benchmark.Benchmark(oneseries)
    runner = None

    with pytest.raises(ValueError):
        runner = IbReadRunner(benchmark, config, 'idfile')

    assert runner is None
