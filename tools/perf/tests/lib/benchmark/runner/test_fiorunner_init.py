#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021, Intel Corporation
#

"""test_fiorunner_init.py -- lib.benchmark.runner.FioRunner init tests"""

import pytest

import shutil

import lib.benchmark
import lib.benchmark.runner
import lib.benchmark.runner.fio
from lib.benchmark.base import Benchmark
from lib.remote_cmd import RemoteCmd

from lib.benchmark.runner.fio import FioRunner

def test_FioRunner_init(monkeypatch):
    """test proper initialization of FioRunner object with all mandatory param"""
    def which_mock(path: str) ->str:
        assert path == '/tmp/fio'
        return path
    monkeypatch.setattr(shutil, 'which', which_mock)
    
    def run_sync_mock(arg1, arg2) ->RemoteCmd:
        remoteCmd = RemoteCmd(None, None, None, exit_status = 0)
        return remoteCmd
    monkeypatch.setattr(RemoteCmd, 'run_sync', run_sync_mock)
    
    oneseries = {'tool': 'fio', 'mode': 'lat', 'tool_mode': 'apm',
                'rw': 'readwrite', 'busy_wait_polling': True, 
                'filetype': 'malloc',
                'requirements' : {'direct_write_to_pmem': True}}
    config = {'server_ip':'server_ip', 'FIO_PATH' : '/tmp'}

    benchmark = lib.benchmark.Benchmark(oneseries)
    runner = FioRunner(benchmark,config, 'idfile')

    assert runner._benchmark == benchmark
    assert runner._config == config
    assert runner._tool == oneseries['tool']
    assert runner._tool_mode == oneseries['tool_mode']
    assert runner._mode == oneseries['mode']
    assert runner._config['server_ip'] == 'server_ip'
    assert runner._benchmark.oneseries['tool'] == 'fio'
    assert runner._benchmark.oneseries['tool_mode'] == 'apm'
    assert runner._benchmark.oneseries['mode'] == 'lat'

def test_FioRunner_init_oneserises_no_tool():
    """failed initialization of FioRunner object - no tool param provided """
    oneseries = {}
    config = {'server_ip':'server_ip', 'FIO_PATH' : '/tmp'}
    benchmark = lib.benchmark.Benchmark(oneseries)

    with pytest.raises(ValueError):
        runner = FioRunner(benchmark,config, 'idfile')

def test_FioRunner_init_oneserises_no_toolmode():
    """failed initialization of FioRunner object - no tool_mode param provided """
    oneseries = {'tool': 'fio'}
    config = {'server_ip':'server_ip', 'FIO_PATH' : '/tmp'}
    benchmark = lib.benchmark.Benchmark(oneseries)

    with pytest.raises(ValueError):
        runner = FioRunner(benchmark,config, 'idfile')

def test_FioRunner_init_oneserises_no_mode():
    """failed initialization of FioRunner object - no mode param provided """
    oneseries = {'tool': 'fio', 'tool_mode': 'apm'}
    config = {'server_ip':'server_ip', 'FIO_PATH' : '/tmp'}
    benchmark = lib.benchmark.Benchmark(oneseries)

    with pytest.raises(ValueError):
        runner = FioRunner(benchmark,config, 'idfile')

def test_FioRunner_init_oneserises_no_rw():
    """failed initialization of FioRunner object - no mode rw provided """
    oneseries = {'tool': 'fio', 'tool_mode': 'apm', 'mode' : 'lat'}
    config = {'server_ip':'server_ip', 'FIO_PATH' : '/tmp'}
    benchmark = lib.benchmark.Benchmark(oneseries)

    with pytest.raises(ValueError):
        runner = FioRunner(benchmark,config, 'idfile')

def test_FioRunner_init_oneserises_no_filetype():
    """failed initialization of FioRunner object - no mode rw provided """
    oneseries = {'tool': 'fio', 'tool_mode': 'apm', 'mode' : 'lat'}
    config = {'server_ip':'server_ip', 'FIO_PATH' : '/tmp'}
    benchmark = lib.benchmark.Benchmark(oneseries)

    with pytest.raises(ValueError):
        runner = FioRunner(benchmark,config, 'idfile')

def test_FioRunner_init_oneserises_no_requirements():
    """failed initialization of FioRunner object - no requirements provided """
    oneseries = {'tool': 'fio', 'tool_mode': 'apm', 'mode': 'lat', 
                'rw':'readwrite', 'filetype': 'malloc'}
    config = {'server_ip':'server_ip', 'FIO_PATH' : '/tmp'}
    benchmark = lib.benchmark.Benchmark(oneseries)

    with pytest.raises(ValueError):
        runner = FioRunner(benchmark,config, 'idfile')

def test_FioRunner_init_oneserises_no_requirements_direct_write_to_pmem():
    """failed initialization of FioRunner object - no requirements provided """
    oneseries = {'tool': 'fio', 'tool_mode': 'apm', 'mode': 'lat', 
                'rw':'readwrite',  'filetype': 'malloc',
                'requirements' : {}}
    config = {'server_ip':'server_ip', 'FIO_PATH' : '/tmp'}
    benchmark = lib.benchmark.Benchmark(oneseries)

    with pytest.raises(ValueError):
        runner = FioRunner(benchmark,config, 'idfile')

def test_FioRunner_init_no_config():
    """failed initialization of FioRunner object - no config provided """
    oneseries = {'tool': 'fio', 'tool_mode': 'apm', 'mode': 'lat', 
                'rw':'readwrite',  'filetype': 'malloc',
                'requirements' : {}}
    benchmark = lib.benchmark.Benchmark(oneseries)

    with pytest.raises(AttributeError):
        runner = FioRunner(benchmark,None, 'idfile')

def test_FioRunner_init_config_no_fio_path():
    """failed initialization of FioRunner object -
        - no server_ip in config provided """
    oneseries = {'tool': 'fio', 'tool_mode': 'apm', 'mode': 'lat', 
                'rw':'readwrite',  'filetype': 'malloc',
                'requirements' : {'direct_write_to_pmem': True}}
    config = {}
    benchmark = lib.benchmark.Benchmark(oneseries)

    with pytest.raises(ValueError):
        runner = FioRunner(benchmark,config, 'idfile')

def test_FioRunner_init_config_no_server_ip(monkeypatch):
    """failed initialization of FioRunner object -
        - no server_ip in config provided """
    def which_mock(path: str) ->str:
        assert path == '/tmp/fio'
        return path
    monkeypatch.setattr(shutil, 'which', which_mock)

    oneseries = {'tool': 'fio', 'tool_mode': 'apm', 'mode': 'lat', 
                'rw':'readwrite',  'filetype': 'malloc',
                'requirements' : {'direct_write_to_pmem': True}}
    config = {'FIO_PATH' : '/tmp'}
    benchmark = lib.benchmark.Benchmark(oneseries)

    with pytest.raises(ValueError) as excinfo:
        runner = FioRunner(benchmark,config, 'idfile')
