#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021, Intel Corporation
#

"""test_fio_runner_init.py
   -- lib.benchmark.runner.fio.FioRunner init tests"""

import shutil
import pytest

import lib.benchmark
import lib.benchmark.runner
import lib.benchmark.runner.fio

from lib.remote_cmd import RemoteCmd
from lib.benchmark.runner.fio import FioRunner

CONFIG_MINI = {'server_ip': 'server_ip', 'FIO_PATH': '/tmp'}

def which_mock(path: str) ->str:
    """mock of shutil.which()"""
    assert path == '/tmp/fio'
    return path

def test_fio_runner_init(monkeypatch):
    """test proper initialization of FioRunner object
       with all mandatory parameters"""
    def run_sync_mock(_arg1, _arg2) -> RemoteCmd:
        """mock of RemoteCmd.run_sync()"""
        return RemoteCmd(None, None, None, exit_status=0)

    def run_mock(_self) -> None:
        """mock of IbReadRunner.run()"""

    monkeypatch.setattr(shutil, 'which', which_mock)
    monkeypatch.setattr(RemoteCmd, 'run_sync', run_sync_mock)
    monkeypatch.setattr(FioRunner, 'run', run_mock)

    oneseries = {'tool': 'fio', 'mode': 'lat', 'tool_mode': 'apm',
                 'rw': 'readwrite', 'busy_wait_polling': True,
                 'filetype': 'malloc',
                 'requirements': {'direct_write_to_pmem': True}}

    benchmark = lib.benchmark.Benchmark(oneseries)
    runner = FioRunner(benchmark, CONFIG_MINI, 'idfile')
    runner.run()

def test_fio_runner_init_oneserises_no_tool():
    """failed initialization of FioRunner object - no tool param provided """
    oneseries = {}
    benchmark = lib.benchmark.Benchmark(oneseries)

    with pytest.raises(ValueError):
        FioRunner(benchmark, CONFIG_MINI, 'idfile')

def test_fio_runner_init_oneserises_no_toolmode():
    """failed initialization of FioRunner object - no tool_mode param provided """
    oneseries = {'tool': 'fio'}
    benchmark = lib.benchmark.Benchmark(oneseries)

    with pytest.raises(ValueError):
        FioRunner(benchmark, CONFIG_MINI, 'idfile')

def test_fio_runner_init_oneserises_no_mode():
    """failed initialization of FioRunner object - no mode param provided """
    oneseries = {'tool': 'fio', 'tool_mode': 'apm'}
    benchmark = lib.benchmark.Benchmark(oneseries)

    with pytest.raises(ValueError):
        FioRunner(benchmark, CONFIG_MINI, 'idfile')

def test_fio_runner_init_oneserises_no_rw():
    """failed initialization of FioRunner object - no mode rw provided """
    oneseries = {'tool': 'fio', 'tool_mode': 'apm', 'mode': 'lat'}
    benchmark = lib.benchmark.Benchmark(oneseries)

    with pytest.raises(ValueError):
        FioRunner(benchmark, CONFIG_MINI, 'idfile')

def test_fio_runner_init_oneserises_no_filetype():
    """failed initialization of FioRunner object - no filetype provided """
    oneseries = {'tool': 'fio', 'tool_mode': 'apm', 'mode': 'lat',
                 'rw': 'readwrite'}
    benchmark = lib.benchmark.Benchmark(oneseries)

    with pytest.raises(ValueError):
        FioRunner(benchmark, CONFIG_MINI, 'idfile')

def test_fio_runner_init_oneserises_no_requirements():
    """failed initialization of FioRunner object - no requirements provided """
    oneseries = {'tool': 'fio', 'tool_mode': 'apm', 'mode': 'lat',
                 'rw': 'readwrite', 'filetype': 'malloc'}
    benchmark = lib.benchmark.Benchmark(oneseries)

    with pytest.raises(ValueError):
        FioRunner(benchmark, CONFIG_MINI, 'idfile')

def test_fio_runner_init_oneserises_no_requirements_direct_write_to_pmem():
    """failed initialization of FioRunner object
       - no direct_write_to_pmem provided"""
    oneseries = {'tool': 'fio', 'tool_mode': 'apm', 'mode': 'lat',
                 'rw': 'readwrite', 'filetype': 'malloc',
                 'requirements': {}}
    benchmark = lib.benchmark.Benchmark(oneseries)

    with pytest.raises(ValueError):
        FioRunner(benchmark, CONFIG_MINI, 'idfile')

def test_fio_runner_init_no_config():
    """failed initialization of FioRunner object - no config provided """
    oneseries = {'tool': 'fio', 'tool_mode': 'apm', 'mode': 'lat',
                 'rw': 'readwrite', 'filetype': 'malloc',
                 'requirements': {'direct_write_to_pmem': True}}
    benchmark = lib.benchmark.Benchmark(oneseries)

    with pytest.raises(AttributeError):
        FioRunner(benchmark, None, 'idfile')

def test_fio_runner_init_config_no_fio_path():
    """failed initialization of FioRunner object -
       - no server_ip in config provided """
    oneseries = {'tool': 'fio', 'tool_mode': 'apm', 'mode': 'lat',
                 'rw': 'readwrite', 'filetype': 'malloc',
                 'requirements': {'direct_write_to_pmem': True}}
    config = {}
    benchmark = lib.benchmark.Benchmark(oneseries)

    with pytest.raises(ValueError):
        FioRunner(benchmark, config, 'idfile')

def test_fio_runner_init_config_no_server_ip(monkeypatch):
    """failed initialization of FioRunner object -
       - no server_ip in config provided """
    monkeypatch.setattr(shutil, 'which', which_mock)

    oneseries = {'tool': 'fio', 'tool_mode': 'apm', 'mode': 'lat',
                 'rw': 'readwrite', 'filetype': 'malloc',
                 'requirements': {'direct_write_to_pmem': True}}
    config = {'FIO_PATH': '/tmp'}
    benchmark = lib.benchmark.Benchmark(oneseries)

    with pytest.raises(ValueError):
        FioRunner(benchmark, config, 'idfile')
