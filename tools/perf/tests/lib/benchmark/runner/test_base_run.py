#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021, Intel Corporation
#

"""test_base_run.py -- lib.benchmark.runner.BaseRunner.run() tests"""

import os
import pytest
import re
import shutil
import subprocess

import lib.benchmark

IP_DUMMY = '101.102.103.104'
CONFIG_DUMMY = {'server_ip': IP_DUMMY}

TOOL_IB_READ = 'ib_read'
TOOL_RPMA_FIO_BENCH = 'fio'

# by-design different to CONFIG_BIG
ENV_MOCK = {
    'server_ip': '0.0.0.0',
    'JOB_NUMA': 1,
    'AUX_PARAMS': '',
    'IB_PATH': '',
    'FIO_PATH': '',
    'REMOTE_USER': '',
    'REMOTE_PASS': '',
    'REMOTE_JOB_NUMA': 1,
    'REMOTE_AUX_PARAMS': '',
    'REMOTE_IB_PATH': '',
    'REMOTE_SUDO_NOPASSWD': False,
    'REMOTE_RNIC_PCIE_ROOT_PORT': '',
    'REMOTE_DIRECT_WRITE_TO_PMEM': False,
    'FORCE_REMOTE_DIRECT_WRITE_TO_PMEM': False,
    'REMOTE_FIO_PATH': '',
    'REMOTE_JOB_PATH': '',
    'REMOTE_JOB_MEM_PATH': '',
    'BUSY_WAIT_POLLING': True,
    'CPU_LOAD_RANGE': '',
    'REMOTE_ANOTHER_NUMA': 0,
    'REMOTE_RESULTS_DIR': '',
    'OUTPUT_FILE': '',
    'REMOTE_CMD_PRE': 'remote cmd pre',
    'REMOTE_CMD_POST': 'remote cmd post',
    'SHORT_RUNTIME': True,
    'TRACER': 'gdbserver localhost:2345',
    'REMOTE_TRACER': 'gdbserver localhost:2345',
    'DO_NOTHING': True,
    'DUMP_CMDS': True
}

CONFIG_BIG = {
    'server_ip': IP_DUMMY,
    'CORES_PER_SOCKET': 1,
    'JOB_NUMA': 0,
    'AUX_PARAMS': '-d mlx5_0 -R',
    'IB_PATH': '/local/ib/path',
    'FIO_PATH': '/local/fio/path',
    'REMOTE_USER': 'user',
    'REMOTE_PASS': 'pass',
    'REMOTE_JOB_NUMA': 0,
    'REMOTE_AUX_PARAMS': '-d mlx5_0 -R',
    'REMOTE_IB_PATH': 'path',
    'REMOTE_SUDO_NOPASSWD': True,
    'REMOTE_RNIC_PCIE_ROOT_PORT': '0000:17:00.0',
    'REMOTE_DIRECT_WRITE_TO_PMEM': True,
    'FORCE_REMOTE_DIRECT_WRITE_TO_PMEM': True,
    'REMOTE_FIO_PATH': '/remote/fio/path',
    'REMOTE_JOB_PATH': '/remote/job/path',
    'REMOTE_JOB_MEM_PATH': '/remote/mem/path',
    'BUSY_WAIT_POLLING': True,
    'CPU_LOAD_RANGE': '00_99',
    'REMOTE_ANOTHER_NUMA': 1,
    'REMOTE_RESULTS_DIR': '/remote/results/path',
    'OUTPUT_FILE': 'output_file.csv',
    'REMOTE_CMD_PRE': 'remote cmd pre',
    'REMOTE_CMD_POST': 'remote cmd post',
    'SHORT_RUNTIME': False,
    'TRACER': 'gdbserver localhost:2345',
    'REMOTE_TRACER': 'gdbserver localhost:2345',
    'DO_NOTHING': False,
    'DUMP_CMDS': False
}

def which_mock(path):
    return path

def run_sync_mock(_config, _cmd):
    return lib.remote_cmd.RemoteCmd(None, None, None, 0)

def run_mock_pass(_self):
    pass

def run_mock_fail(_self):
    func = re.match('\S+', str(_self)).group(0)[1:]
    assert False, '{}.run() should not be called'.format(func)

def test_ib_read_success(benchmark_ib_read, tmpdir, monkeypatch):
    """a test of IbReadRunner__init"""
    monkeypatch.setattr(shutil, 'which', which_mock)
    monkeypatch.setattr(lib.remote_cmd.RemoteCmd, 'run_sync', run_sync_mock)
    monkeypatch.setattr(lib.benchmark.runner.ib_read.IbReadRunner, 'run', run_mock_pass)
    lib.benchmark.runner.BaseRunner.run(benchmark_ib_read, CONFIG_BIG, str(tmpdir))
    # marking a benchmark as 'done' is done outside of the runner
    assert not benchmark_ib_read.is_done()

def test_fio_success(benchmark_fio, tmpdir, monkeypatch):
    """a test of FioRunner__init"""
    monkeypatch.setattr(shutil, 'which', which_mock)
    monkeypatch.setattr(lib.remote_cmd.RemoteCmd, 'run_sync', run_sync_mock)
    monkeypatch.setattr(lib.benchmark.runner.fio.FioRunner, 'run', run_mock_pass)
    lib.benchmark.runner.BaseRunner.run(benchmark_fio, CONFIG_BIG, str(tmpdir))
    # marking a benchmark as 'done' is done outside of the runner
    assert not benchmark_fio.is_done()

@pytest.mark.parametrize('key', ['rw', 'filetype'])
def test_ib_read_missing_key(oneseries_ib_read, key, tmpdir, monkeypatch):
    """a test of missing key in IbReadRunner__init"""
    monkeypatch.setattr(shutil, 'which', which_mock)
    monkeypatch.setattr(lib.remote_cmd.RemoteCmd, 'run_sync', run_sync_mock)
    monkeypatch.setattr(lib.benchmark.runner.ib_read.IbReadRunner, 'run', run_mock_fail)
    oneseries = {**oneseries_ib_read}
    oneseries.pop(key)
    benchmark = lib.benchmark.Benchmark(oneseries)
    with pytest.raises(ValueError):
        lib.benchmark.runner.BaseRunner.run(benchmark, CONFIG_BIG, str(tmpdir))
    # marking a benchmark as 'done' is done outside of the runner
    assert not benchmark.is_done()

@pytest.mark.parametrize('key', ['tool_mode', 'mode', 'rw'])
def test_fio_missing_key(oneseries_fio, key, tmpdir, monkeypatch):
    """a test of missing key in FioRunner__init"""
    monkeypatch.setattr(shutil, 'which', which_mock)
    monkeypatch.setattr(lib.remote_cmd.RemoteCmd, 'run_sync', run_sync_mock)
    monkeypatch.setattr(lib.benchmark.runner.fio.FioRunner, 'run', run_mock_fail)
    oneseries = {**oneseries_fio}
    oneseries.pop(key)
    benchmark = lib.benchmark.Benchmark(oneseries)
    with pytest.raises(ValueError):
        lib.benchmark.runner.BaseRunner.run(benchmark, CONFIG_BIG, str(tmpdir))
    # marking a benchmark as 'done' is done outside of the runner
    assert not benchmark.is_done()

@pytest.mark.parametrize('key', ['rw', 'filetype'])
def test_ib_read_wrong_value(oneseries_ib_read, key, tmpdir, monkeypatch):
    """a test of missing key in IbReadRunner__init"""
    monkeypatch.setattr(shutil, 'which', which_mock)
    monkeypatch.setattr(lib.remote_cmd.RemoteCmd, 'run_sync', run_sync_mock)
    monkeypatch.setattr(lib.benchmark.runner.ib_read.IbReadRunner, 'run', run_mock_fail)
    oneseries = {**oneseries_ib_read}
    oneseries[key] = 'wrong_value'
    benchmark = lib.benchmark.Benchmark(oneseries)
    with pytest.raises(ValueError):
        lib.benchmark.runner.BaseRunner.run(benchmark, CONFIG_BIG, str(tmpdir))
    # marking a benchmark as 'done' is done outside of the runner
    assert not benchmark.is_done()

@pytest.mark.parametrize('key', ['tool_mode', 'mode', 'rw'])
def test_fio_wrong_value(oneseries_fio, key, tmpdir, monkeypatch):
    """a test of missing key in FioRunner__init"""
    monkeypatch.setattr(shutil, 'which', which_mock)
    monkeypatch.setattr(lib.remote_cmd.RemoteCmd, 'run_sync', run_sync_mock)
    monkeypatch.setattr(lib.benchmark.runner.fio.FioRunner, 'run', run_mock_fail)
    oneseries = {**oneseries_fio}
    oneseries[key] = 'wrong_value'
    benchmark = lib.benchmark.Benchmark(oneseries)
    with pytest.raises(ValueError):
        lib.benchmark.runner.BaseRunner.run(benchmark, CONFIG_BIG, str(tmpdir))
    # marking a benchmark as 'done' is done outside of the runner
    assert not benchmark.is_done()
