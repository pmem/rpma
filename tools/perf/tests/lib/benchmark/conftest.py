#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021, Intel Corporation
#

"""conftest.py -- a common for lib.benchmark.* tests"""

import pytest
import lib.benchmark

ONESERIES_DUMMY = \
    {'tool': 'dummy', 'mode': 'dummy', 'filetype': 'malloc', 'id': 'di'}
ONESERIES_BASH = \
    {**ONESERIES_DUMMY, **{'tool': 'tool.sh'}}
ONESERIES_EXECUTOR = \
    {**ONESERIES_DUMMY, **{'tool': 'tool'}}
ONESERIES_IB_READ = \
    {**ONESERIES_DUMMY, **{'tool': 'ib_read', 'mode': 'lat', 'rw': 'read'}}
ONESERIES_FIO = \
    {**ONESERIES_DUMMY, **{'tool': 'fio', 'tool_mode': 'apm', 'mode': 'lat',
    'rw': 'read', 'requirements': {'direct_write_to_pmem': True}}}

@pytest.fixture(scope='function')
def oneseries_dummy():
    """provide a dummy oneseries"""
    return ONESERIES_DUMMY.copy()

@pytest.fixture(scope='function')
def oneseries_bash():
    """provide a bash oneseries"""
    return ONESERIES_BASH.copy()

@pytest.fixture(scope='function')
def oneseries_executor():
    """provide an executor oneseries"""
    return ONESERIES_EXECUTOR.copy()

@pytest.fixture(scope='function')
def oneseries_ib_read():
    """provide an ib_read oneseries"""
    return ONESERIES_IB_READ.copy()

@pytest.fixture(scope='function')
def oneseries_fio():
    """provide a fio oneseries"""
    return ONESERIES_FIO.copy()

@pytest.fixture(scope='function')
def benchmark_dummy(oneseries_dummy):
    """create a dummy Benchmark instance"""
    return lib.benchmark.Benchmark(oneseries_dummy)

@pytest.fixture(scope='function')
def benchmark_bash(oneseries_bash):
    """create a bash Benchmark instance"""
    return lib.benchmark.Benchmark(oneseries_bash)

@pytest.fixture(scope='function')
def benchmark_executor(oneseries_executor):
    """create an executor Benchmark instance"""
    return lib.benchmark.Benchmark(oneseries_executor)

@pytest.fixture(scope='function')
def benchmark_ib_read(oneseries_ib_read):
    """create an ib_read Benchmark instance"""
    return lib.benchmark.Benchmark(oneseries_ib_read)

@pytest.fixture(scope='function')
def benchmark_fio(oneseries_fio):
    """create a fio Benchmark instance"""
    return lib.benchmark.Benchmark(oneseries_fio)
