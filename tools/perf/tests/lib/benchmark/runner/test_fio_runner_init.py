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

__CONFIG_FIO = {'server_ip': 'server_ip', 'FIO_PATH': '/tmp'}

@pytest.fixture(scope='function', name='config_fio')
def __config_fio():
    """provide a fio oneseries_fio"""
    return __CONFIG_FIO.copy()

def which_mock(path: str) ->str:
    """mock of shutil.which()"""
    assert path == '/tmp/fio'
    return path

def test_fio_runner_init(oneseries_fio, config_fio, monkeypatch):
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

    benchmark = lib.benchmark.Benchmark(oneseries_fio)
    runner = FioRunner(benchmark, config_fio, 'idfile')
    runner.run()

    #pylint: disable=protected-access
    #pylint: disable=no-member
    assert runner._FioRunner__benchmark == benchmark
    assert runner._FioRunner__config == config_fio
    assert runner._FioRunner__idfile == 'idfile'
    assert runner._FioRunner__tool == oneseries_fio['tool']
    assert runner._FioRunner__tool_mode == oneseries_fio['tool_mode']
    assert runner._FioRunner__mode == oneseries_fio['mode']
    #pylint: enable=no-member
    #pylint: enable=protected-access

@pytest.mark.parametrize('key', ['tool', 'tool_mode', 'mode', 'rw', \
                         'filetype', 'requirements'])
def test_fio_runner_init_oneserises_incomplete(oneseries_fio, config_fio, key):
    """failed initialization of FioRunner object - incomlete oneseries_fio """
    oneseries_fio.pop(key)
    benchmark = lib.benchmark.Benchmark(oneseries_fio)

    with pytest.raises(ValueError):
        FioRunner(benchmark, config_fio, 'idfile')

def test_fio_runner_init_no_direct_write_to_pmem(oneseries_fio, config_fio):
    """failed initialization of FioRunner object
       - no direct_write_to_pmem provided"""
    oneseries_fio.pop('requirements')
    oneseries_fio['requirements'] = {}

    benchmark = lib.benchmark.Benchmark(oneseries_fio)

    with pytest.raises(ValueError):
        FioRunner(benchmark, config_fio, 'idfile')

def test_fio_runner_init_no_config(oneseries_fio):
    """failed initialization of FioRunner object - no config provided """
    benchmark = lib.benchmark.Benchmark(oneseries_fio)

    with pytest.raises(AttributeError):
        FioRunner(benchmark, None, 'idfile')

@pytest.mark.parametrize('key', ['server_ip', 'FIO_PATH'])
def test_fio_runner_init_config_incomplete(oneseries_fio, config_fio, key):
    """failed initialization of FioRunner object -
       - no fio_path in config provided """
    config_fio.pop(key)
    benchmark = lib.benchmark.Benchmark(oneseries_fio)
    with pytest.raises(ValueError):
        FioRunner(benchmark, config_fio, 'idfile')

@pytest.mark.parametrize('key', ['tool', 'tool_mode', 'mode', 'rw',
                                 'filetype'])
def test_fio_runner_init_wrong_value(oneseries_fio, config_fio, key):
    """failed initialization of IbReadRunner object -
       - no fio_path in config provided """
    oneseries_fio.pop(key)
    oneseries_fio[key] = 'an incorrect value'
    benchmark = lib.benchmark.Benchmark(oneseries_fio)
    with pytest.raises(ValueError):
        FioRunner(benchmark, config_fio, 'idfile')
