#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021-2022, Intel Corporation
#

"""test_executor_run.py -- lib.benchmark.runner.Executor.run() tests"""

import shutil
import pytest

import lib.benchmark

CONFIG = {
    'SERVER_IP': '101.102.103.104'
}

def which_mock(path):
    """mock of shutil.which()"""
    return path

def run_sync_mock(_config, _cmd):
    """mock of RemoteCmd.run_sync()"""
    return lib.remote_cmd.RemoteCmd(None, None, None, 0)

def run_mock_pass(_self):
    """passing mock of *Runner.run()"""

def test_success_ib_read(benchmark_ib_read, tmpdir, monkeypatch):
    """a test of IbReadRunner__init"""
    monkeypatch.setattr(shutil, 'which', which_mock)
    monkeypatch.setattr(lib.remote_cmd.RemoteCmd, 'run_sync', run_sync_mock)
    monkeypatch.setattr(lib.benchmark.runner.ib_read.IbReadRunner, 'run',
                        run_mock_pass)
    lib.benchmark.runner.Executor.run(benchmark_ib_read, CONFIG, str(tmpdir))
    # marking a benchmark as 'done' is done outside of the runner
    assert not benchmark_ib_read.is_done()

def test_success_fio(benchmark_fio, tmpdir, monkeypatch):
    """a test of FioRunner__init"""
    monkeypatch.setattr(shutil, 'which', which_mock)
    monkeypatch.setattr(lib.remote_cmd.RemoteCmd, 'run_sync', run_sync_mock)
    monkeypatch.setattr(lib.benchmark.runner.fio.FioRunner, 'run',
                        run_mock_pass)
    lib.benchmark.runner.Executor.run(benchmark_fio, CONFIG, str(tmpdir))
    # marking a benchmark as 'done' is done outside of the runner
    assert not benchmark_fio.is_done()

@pytest.mark.parametrize('tool', [None, 'Unknown runner'])
def test_wrong_tool(tool, tmpdir):
    """a test of wrong 'tool' key in oneseries"""
    oneseries = {'tool': tool}
    benchmark = lib.benchmark.Benchmark(oneseries)
    with pytest.raises(NotImplementedError):
        lib.benchmark.runner.Executor.run(benchmark, CONFIG, str(tmpdir))
    # marking a benchmark as 'done' is done outside of the runner
    assert not benchmark.is_done()

def test_missing_tool(tmpdir):
    """a test of missing 'tool' key in oneseries"""
    oneseries = {}
    benchmark = lib.benchmark.Benchmark(oneseries)
    with pytest.raises(ValueError):
        lib.benchmark.runner.Executor.run(benchmark, CONFIG, str(tmpdir))
    # marking a benchmark as 'done' is done outside of the runner
    assert not benchmark.is_done()
