#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021, Intel Corporation
#

"""test_ib_read_runner_init.py -- lib.benchmark.runner.ib_read init tests"""

import pytest
import shutil

import lib.benchmark
import lib.benchmark.runner
import lib.benchmark.runner.ib_read
from lib.benchmark.base import Benchmark
from lib.remote_cmd import RemoteCmd

from lib.benchmark.runner.ib_read import IbReadRunner

def test_IbReadRunner_init(monkeypatch):
    """test proper initialization of IbReadRunner object with all mandatory param"""
    def which_mock(path: str) ->str:
        assert path == 'ib_read_lat'
        return path
    monkeypatch.setattr(shutil, 'which', which_mock)

    def run_sync_mock(arg1, arg2) ->RemoteCmd:
        remoteCmd = RemoteCmd(None, None, None, exit_status = 0)
        return remoteCmd
    monkeypatch.setattr(RemoteCmd, 'run_sync', run_sync_mock)

    oneseries = {'tool': 'ib_read', 'tool_mode': 'lat', 'mode': 'lat',
                'rw': 'read', 'filetype': 'malloc',
                'requirements' : {'direct_write_to_pmem': True}}
    config = {'server_ip':'server_ip'}

    benchmark = lib.benchmark.Benchmark(oneseries)
    runner = IbReadRunner(benchmark,config, 'idfile')
    assert runner._benchmark == benchmark
    assert runner._config == config
    assert runner._idfile == 'idfile'


def test_IbReadRunner_init_oneserises_no_tool():
    """failed initialization of IbReadRunner object - no tool param provided """
    oneseries = {}
    config = {'server_ip':'server_ip'}
    benchmark = lib.benchmark.Benchmark(oneseries)
    with pytest.raises(ValueError):
        runner = IbReadRunner(benchmark,config, 'idfile')

def test_IbReadRunner_init_oneserises_no_toolmode():
    """failed initialization of IbReadRunner object - no tool_mode param provided """
    oneseries = {'tool': 'ib_read'}
    config = {'server_ip':'server_ip'}
    benchmark = lib.benchmark.Benchmark(oneseries)

    with pytest.raises(ValueError):
        runner = IbReadRunner(benchmark,config, 'idfile')

def test_IbReadRunner_init_oneserises_no_mode():
    """failed initialization of IbReadRunner object - no mode param provided """
    oneseries = {'tool': 'ib_read'}
    config = {'server_ip':'server_ip'}
    benchmark = lib.benchmark.Benchmark(oneseries)

    with pytest.raises(ValueError):
        runner = IbReadRunner(benchmark,config, 'idfile')

def test_IbReadRunner_init_oneserises_no_rw():
    """failed initialization of IbReadRunner object - no mode rw provided """
    oneseries = {'tool': 'ib_read', 'mode' : 'lat'}
    config = {'server_ip':'server_ip'}
    benchmark = lib.benchmark.Benchmark(oneseries)

    with pytest.raises(ValueError):
        runner = IbReadRunner(benchmark,config, 'idfile')

def test_IbReadRunner_init_oneserises_no_filetype():
    """failed initialization of IbReadRunner object - no mode rw provided """
    oneseries = {'tool': 'ib_read', 'tool_mode' : 'lat', 'mode' : 'lat', 'rw':'read', }
    config = {'server_ip':'server_ip'}
    benchmark = lib.benchmark.Benchmark(oneseries)

    with pytest.raises(ValueError):
        runner = IbReadRunner(benchmark,config, 'idfile')

def test_IbReadRunner_init_no_config():
    """failed initialization of IbReadRunner object - no config provided """
    oneseries = {'tool': 'ib_read', 'tool_mode': 'lat', 'mode': 'lat',
                'rw':'read',  'filetype': 'malloc',
                'requirements' : {}}
    benchmark = lib.benchmark.Benchmark(oneseries)

    with pytest.raises(RuntimeError):
        runner = IbReadRunner(benchmark,None, 'idfile')

def test_IbReadRunner_init_config_no_server_ip(monkeypatch):
    """failed initialization of IbReadRunner object -
        - no server_ip in config provided """
    def which_mock(path: str) ->str:
        assert path == 'ib_read_lat'
        return path
    monkeypatch.setattr(shutil, 'which', which_mock)


    oneseries = {'tool': 'ib_read', 'mode': 'lat', 
                'rw':'read',  'filetype': 'malloc'}
    config = {}
    benchmark = lib.benchmark.Benchmark(oneseries)
    with pytest.raises(ValueError) as excinfo:
        runner = IbReadRunner(benchmark,config, 'idfile')
    
