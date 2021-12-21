#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021, Intel Corporation
#

"""test_run.py -- lib.benchmark.Benchmark.run() tests"""

import pytest

import lib.benchmark
import lib.common

IP_DUMMY = '101.102.103.104'
CONFIG_DEFAULT = {'server_ip': IP_DUMMY}
CONFIG_DUMMY = {**CONFIG_DEFAULT, 'dummy_results': True}

@pytest.mark.parametrize('key', ['tool', 'mode', 'id', 'filetype'])
def test_incomplete_benchmark(oneseries_dummy, key, tmpdir):
    """an incomplete benchmark definition"""
    oneseries_dummy.pop(key, None)
    benchmark = lib.benchmark.Benchmark(oneseries_dummy)
    with pytest.raises(ValueError):
        benchmark.run(CONFIG_DUMMY, str(tmpdir))
    assert not benchmark.is_done()

def test_no_server_ip(benchmark_dummy, tmpdir):
    """a config without 'server_ip'"""
    config = {**CONFIG_DUMMY}
    config.pop('server_ip', None)
    with pytest.raises(ValueError):
        benchmark_dummy.run(config, str(tmpdir))
    assert not benchmark_dummy.is_done()

def test_dummy_runner(benchmark_dummy, tmpdir, monkeypatch):
    """a simple Dummy runner call"""
    run_mock_used = False
    def run_mock(_self, _config, idfile):
        nonlocal run_mock_used, tmpdir
        assert idfile == output
        run_mock_used = True
    monkeypatch.setattr(lib.benchmark.base.Dummy, 'run', run_mock)
    output = lib.benchmark.get_result_path(str(tmpdir),
                                           benchmark_dummy.identifier)
    benchmark_dummy.run(CONFIG_DUMMY, str(tmpdir))
    assert run_mock_used
    assert benchmark_dummy.is_done()

def test_bash_runner(benchmark_bash, tmpdir, monkeypatch):
    """a simple Bash runner call"""
    run_mock_used = False
    def run_mock(_self, _config, idfile):
        nonlocal run_mock_used, tmpdir
        assert idfile == output
        run_mock_used = True
    monkeypatch.setattr(lib.benchmark.base.Bash, 'run', run_mock)
    output = lib.benchmark.get_result_path(str(tmpdir),
                                           benchmark_bash.identifier)
    benchmark_bash.run(CONFIG_DEFAULT, str(tmpdir))
    assert run_mock_used
    assert benchmark_bash.is_done()

def test_base_runner(benchmark_base, tmpdir, monkeypatch):
    """a simple base runner call"""
    run_mock_used = False
    def run_mock(_self, _config, idfile):
        nonlocal run_mock_used, tmpdir
        assert idfile == output
        run_mock_used = True
    monkeypatch.setattr(lib.benchmark.base.BaseRunner, 'run', run_mock)
    output = lib.benchmark.get_result_path(str(tmpdir),
                                           benchmark_base.identifier)
    benchmark_base.run(CONFIG_DEFAULT, str(tmpdir))
    assert run_mock_used
    assert benchmark_base.is_done()
