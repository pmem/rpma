#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021, Intel Corporation
#

"""test_ib_read_runner_init.py
   -- lib.benchmark.runner.ib_read.IbReadRunner init tests"""

import shutil
import pytest

import lib.benchmark
import lib.benchmark.runner
import lib.benchmark.runner.ib_read

from lib.remote_cmd import RemoteCmd
from lib.benchmark.runner.ib_read import IbReadRunner

__CONFIG_IB_READ = {'server_ip': 'server_ip'}

@pytest.fixture(scope='function', name='config_ib_read')
def __config_ib_read():
    """provide a fio oneseries"""
    return __CONFIG_IB_READ.copy()

def which_mock(path: str) -> str:
    """mock of shutil.which()"""
    assert path == 'ib_read_lat'
    return path

def test_ib_read_runner_init(oneseries_ib_read, config_ib_read, monkeypatch):
    """test proper initialization of IbReadRunner object
       with all mandatory parameters"""
    def run_sync_mock(_arg1, _arg2) -> RemoteCmd:
        """mock of RemoteCmd.run_sync()"""
        return RemoteCmd(None, None, None, exit_status=0)

    def run_mock(_self) -> None:
        """mock of IbReadRunner.run()"""

    monkeypatch.setattr(shutil, 'which', which_mock)
    monkeypatch.setattr(RemoteCmd, 'run_sync', run_sync_mock)
    monkeypatch.setattr(IbReadRunner, 'run', run_mock)

    benchmark = lib.benchmark.Benchmark(oneseries_ib_read)
    runner = IbReadRunner(benchmark, config_ib_read, 'idfile')
    runner.run()

    #pylint: disable=protected-access
    #pylint: disable=no-member
    assert runner._IbReadRunner__benchmark == benchmark
    assert runner._IbReadRunner__config == config_ib_read
    assert runner._IbReadRunner__idfile == 'idfile'
    assert runner._IbReadRunner__tool == oneseries_ib_read['tool']
    assert runner._IbReadRunner__mode == oneseries_ib_read['mode']
    #pylint: enable=no-member
    #pylint: enable=protected-access

# XXX 'requirements' and 'tool_mode' are not yet suported by ib_read_runner
# to be added later
#@pytest.mark.parametrize('key', ['tool', 'tool_mode', 'mode', 'rw',
#                                 'filetype', 'requirements'])
@pytest.mark.parametrize('key', ['tool', 'mode', 'rw', 'filetype'])
def test_ib_read_runner_init_oneserises_incomplete(oneseries_ib_read,
        config_ib_read, key):
    """failed initialization of IbReadRunner object - incomlete oneseries"""
    oneseries = {**oneseries_ib_read}
    oneseries.pop(key)
    benchmark = lib.benchmark.Benchmark(oneseries)

    with pytest.raises(ValueError):
        IbReadRunner(benchmark, config_ib_read, 'idfile')

# XXX shall ib_read_runner anyhow control direct_write_to_pmem
@pytest.mark.skip(reason="ib_read does not control direct_write_to_pmem")
def test_ib_read_runner_init_no_direct_write_to_pmem(oneseries_ib_read,
         config_ib_read, monkeypatch):
    """failed initialization of IbReadRunner object
       - no direct_write_to_pmem provided"""
    def run_sync_mock(_arg1, _arg2) -> RemoteCmd:
        """mock of RemoteCmd.run_sync()"""
        return RemoteCmd(None, None, None, exit_status=0)

    monkeypatch.setattr(RemoteCmd, 'run_sync', run_sync_mock)

    oneseries = {**oneseries_ib_read}
    oneseries.pop('requirements')
    oneseries['requirements'] = {}

    benchmark = lib.benchmark.Benchmark(oneseries)

    with pytest.raises(ValueError):
        IbReadRunner(benchmark, config_ib_read, 'idfile')

def test_ib_read_runner_init_no_config(oneseries_ib_read):
    """failed initialization of IbReadRunner object - no config provided"""
    oneseries = {**oneseries_ib_read}
    benchmark = lib.benchmark.Benchmark(oneseries)

    with pytest.raises(AttributeError):
        IbReadRunner(benchmark, None, 'idfile')

@pytest.mark.parametrize('key', ['server_ip'])
def test_ib_read_runner_init_config_incomplete(oneseries_ib_read,
        config_ib_read, key):
    """failed initialization of IbReadRunner object -
       - incomplete config provided """
    oneseries = {**oneseries_ib_read}
    config = {**config_ib_read}
    config.pop(key)
    benchmark = lib.benchmark.Benchmark(oneseries)
    with pytest.raises(ValueError):
        IbReadRunner(benchmark, config, 'idfile')

@pytest.mark.parametrize('key', ['rw', 'filetype', 'tool'])
def test_ib_read_runner_init_wrong_value(oneseries_ib_read,
        config_ib_read, key):
    """failed initialization of IbReadRunner object -
       - invalid value provided"""
    oneseries = {**oneseries_ib_read}
    oneseries.pop(key)
    oneseries[key] = '!@#$%^)(*&dsaf;a;ljka;sdfja'
    benchmark = lib.benchmark.Benchmark(oneseries)
    with pytest.raises(ValueError):
        IbReadRunner(benchmark, config_ib_read, 'idfile')
