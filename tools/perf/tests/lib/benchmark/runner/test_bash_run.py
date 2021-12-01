#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021, Intel Corporation
#

"""test_bash_run.py -- lib.benchmark.runner.Bash.run() tests"""

import os
import subprocess
import pytest

import lib.benchmark

IP_DUMMY = '101.102.103.104'
CONFIG_DUMMY = {'server_ip': IP_DUMMY}

def test_process_success(benchmark_bash, tmpdir, monkeypatch):
    """a basic subprocess test"""
    def run_mock(_args, env, check):
        assert check
    monkeypatch.setattr(subprocess, 'run', run_mock)
    lib.benchmark.runner.Bash.run(benchmark_bash, CONFIG_DUMMY, str(tmpdir))
    # marking a benchmark as 'done' is done outside of the runner
    assert not benchmark_bash.is_done()

def test_process_fail(benchmark_bash, tmpdir, monkeypatch):
    """a negative subprocess test"""
    def run_mock(_args, env, check):
        assert check
        raise subprocess.CalledProcessError(-1, "")
    monkeypatch.setattr(subprocess, 'run', run_mock)
    with pytest.raises(subprocess.CalledProcessError):
        lib.benchmark.runner.Bash.run(benchmark_bash, CONFIG_DUMMY, str(tmpdir))
    # marking a benchmark as 'done' is done outside of the runner
    assert not benchmark_bash.is_done()

TOOL_IB_READ = 'ib_read.sh'
TOOL_RPMA_FIO_BENCH = 'rpma_fio_bench.sh'

@pytest.mark.parametrize('mode',
    ['bw-bs', 'bw-dp-exp', 'bw-dp-lin', 'bw-th', 'lat'])
def test_ib_read(mode, oneseries_bash, tmpdir, monkeypatch):
    """test all arguments variants of ib_read.sh"""
    def run_mock(args, env, check):
        nonlocal output_file
        assert len(args) == 3
        # XXX is it always correct to assume the tool is in the current working
        # directory?
        assert args[0] == './' + TOOL_IB_READ
        assert args[1] == IP_DUMMY
        assert args[2] == mode
        assert env['OUTPUT_FILE'] == output_file
        assert check
    monkeypatch.setattr(subprocess, 'run', run_mock)
    oneseries = {**oneseries_bash, 'tool': TOOL_IB_READ, 'mode': mode}
    benchmark = lib.benchmark.Benchmark(oneseries)
    output_file = lib.benchmark.get_result_path(str(tmpdir),
                                                       benchmark.identifier)
    lib.benchmark.runner.Bash.run(benchmark, CONFIG_DUMMY, output_file)
    # marking a benchmark as 'done' is done outside of the runner
    assert not benchmark.is_done()

@pytest.mark.parametrize('tool_mode',
    ['apm', 'gpspm', 'aof_sw', 'aof_hw'])
@pytest.mark.parametrize('readwrite',
    ['read', 'randread', 'write', 'randwrite', 'rw', 'randrw'])
@pytest.mark.parametrize('mode',
    ['bw-bs', 'bw-dp-exp', 'bw-dp-lin', 'bw-th', 'bw-cpu', 'bw-cpu-mt', 'lat',
        'lat-cpu'])
def test_rpma_fio_bench(tool_mode, readwrite, mode, oneseries_bash, tmpdir, monkeypatch):
    """test all arguments variants of rpma_fio_bench.sh"""
    def run_mock(args, env, check):
        nonlocal output_file
        assert len(args) == 5
        # XXX is it always correct to assume the tool is in the current working
        # directory?
        assert args[0] == './' + TOOL_RPMA_FIO_BENCH
        assert args[1] == IP_DUMMY
        assert args[2] == tool_mode
        assert args[3] == readwrite
        assert args[4] == mode
        assert env['OUTPUT_FILE'] == output_file
        assert check
    monkeypatch.setattr(subprocess, 'run', run_mock)
    oneseries = {**oneseries_bash, 'tool': TOOL_RPMA_FIO_BENCH, 'mode': mode,
        'tool_mode': tool_mode, 'rw': readwrite, 'busy_wait_polling': True}
    benchmark = lib.benchmark.Benchmark(oneseries)
    output_file = lib.benchmark.get_result_path(str(tmpdir),
                                                       benchmark.identifier)
    lib.benchmark.runner.Bash.run(benchmark, CONFIG_DUMMY, output_file)
    # marking a benchmark as 'done' is done outside of the runner
    assert not benchmark.is_done()

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

def test_config_overwrite_env(benchmark_bash, tmpdir, monkeypatch):
    """overwrite environment by config"""
    def run_mock(_args, env, check):
        nonlocal output_file
        for k, _ in CONFIG_BIG.items():
            if k == 'OUTPUT_FILE':
                assert env[k] == output_file
            elif k == 'REMOTE_JOB_MEM_PATH':
                # the config value will be overwritten by the benchmark
                assert env[k] == 'malloc'
            elif isinstance(CONFIG_BIG[k], bool):
                if CONFIG_BIG[k]:
                    assert env[k] == '1'
                else:
                    assert env[k] == '0'
            else:
                assert env[k] == str(CONFIG_BIG[k])
        assert check
    monkeypatch.setattr(subprocess, 'run', run_mock)
    monkeypatch.setattr(os, 'environ', ENV_MOCK)
    output_file = \
        lib.benchmark.get_result_path(str(tmpdir),
                                      benchmark_bash.identifier)
    lib.benchmark.runner.Bash.run(benchmark_bash, CONFIG_BIG, output_file)
    # marking a benchmark as 'done' is done outside of the runner
    assert not benchmark_bash.is_done()

@pytest.mark.parametrize('filetype', ['malloc', 'pmem'])
def test_filetype(filetype, oneseries_bash, tmpdir, monkeypatch):
    """filetype to REMOTE_JOB_MEM_PATH mapping"""
    # sanity check
    assert CONFIG_BIG['REMOTE_JOB_MEM_PATH'] != 'malloc'
    def run_mock(_args, env, check):
        if filetype == 'malloc':
            assert env['REMOTE_JOB_MEM_PATH'] == 'malloc'
        else:
            assert env['REMOTE_JOB_MEM_PATH'] == \
                CONFIG_BIG['REMOTE_JOB_MEM_PATH']
        assert check
    monkeypatch.setattr(subprocess, 'run', run_mock)
    oneseries = {**oneseries_bash, 'filetype': filetype}
    benchmark = lib.benchmark.Benchmark(oneseries)
    lib.benchmark.runner.Bash.run(benchmark, CONFIG_BIG, str(tmpdir))
    # marking a benchmark as 'done' is done outside of the runner
    assert not benchmark.is_done()

@pytest.mark.parametrize('config_remote_job_mem_path', ['malloc', None])
def test_filetype_pmem_no_mem_path(config_remote_job_mem_path, oneseries_bash,
        tmpdir, monkeypatch):
    """filetype=pmem when no REMOTE_JOB_MEM_PATH provided"""
    def run_mock(_args, **_):
        assert False, "subprocess.run() should not be called"
    monkeypatch.setattr(subprocess, 'run', run_mock)
    oneseries = {**oneseries_bash, 'filetype': 'pmem'}
    benchmark = lib.benchmark.Benchmark(oneseries)
    if config_remote_job_mem_path is None:
        CONFIG_BIG.pop('REMOTE_JOB_MEM_PATH', None)
    else:
        CONFIG_BIG['REMOTE_JOB_MEM_PATH'] = config_remote_job_mem_path
    with pytest.raises(ValueError):
        lib.benchmark.runner.Bash.run(benchmark, CONFIG_BIG, str(tmpdir))
    # marking a benchmark as 'done' is done outside of the runner
    assert not benchmark.is_done()

@pytest.mark.parametrize(
    'config_busy_wait_polling,busy_wait_polling,expected_busy_wait_polling',
    [(True, False, '0'), (False, True, '1')])
def test_busy_wait_polling(config_busy_wait_polling, busy_wait_polling,
        expected_busy_wait_polling, oneseries_bash, tmpdir, monkeypatch):
    """busy_wait_polling to BUSY_WAIT_POLLING mapping"""
    def run_mock(_args, env, check):
        assert env['BUSY_WAIT_POLLING'] == expected_busy_wait_polling
        assert check
    monkeypatch.setattr(subprocess, 'run', run_mock)
    oneseries = {**oneseries_bash, 'busy_wait_polling': busy_wait_polling}
    benchmark = lib.benchmark.Benchmark(oneseries)
    config = {**CONFIG_BIG, 'BUSY_WAIT_POLLING': config_busy_wait_polling}
    lib.benchmark.runner.Bash.run(benchmark, config, str(tmpdir))
    # marking a benchmark as 'done' is done outside of the runner
    assert not benchmark.is_done()

@pytest.mark.parametrize('readwrite', ['write', 'randwrite'])
@pytest.mark.parametrize('mode',
    ['bw-bs', 'bw-dp-exp', 'bw-dp-lin', 'bw-th', 'bw-cpu', 'bw-cpu-mt', 'lat',
        'lat-cpu'])
def test_gpspm_no_busy_wait_polling(readwrite, mode, oneseries_bash, tmpdir, monkeypatch):
    """busy_wait_polling is required in all cases when tool_mode=gpspm"""
    def run_mock(_args, **_):
        assert False, "subprocess.run() should not be called"
    monkeypatch.setattr(subprocess, 'run', run_mock)
    oneseries = {**oneseries_bash, 'tool': TOOL_RPMA_FIO_BENCH, 'mode': mode,
        'tool_mode': 'gpspm', 'rw': readwrite}
    benchmark = lib.benchmark.Benchmark(oneseries)
    with pytest.raises(ValueError):
        lib.benchmark.runner.Bash.run(benchmark, CONFIG_BIG, str(tmpdir))
    # marking a benchmark as 'done' is done outside of the runner
    assert not benchmark.is_done()
