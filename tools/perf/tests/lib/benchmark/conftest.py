#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021-2022, Intel Corporation
#

"""conftest.py -- a common for lib.benchmark.* tests"""

import pytest

import lib.benchmark

__ONESERIES_DUMMY = \
    {'tool': 'dummy', 'mode': 'dummy', 'filetype': 'malloc', 'id': 'di'}
__ONESERIES_EXECUTOR = {**__ONESERIES_DUMMY, **{'tool': 'tool'}}

__ONESERIES_FIO = \
    {**__ONESERIES_DUMMY, **{'tool': 'fio', 'tool_mode': 'apm', 'mode': 'lat',
    'rw': 'read', 'busy_wait_polling': False,
    'requirements': {'direct_write_to_pmem': True}}}

__ONESERIES_IB_READ = \
    {**__ONESERIES_DUMMY, **{'tool': 'ib_read', 'tool_mode': 'lat',
    'mode': 'lat', 'rw': 'read', 'filetype': 'malloc',
    'requirements': {'direct_write_to_pmem': True}}}

@pytest.fixture(scope='function', name='oneseries_dummy')
def __oneseries_dummy():
    """provide a oneseries dummy"""
    return __ONESERIES_DUMMY.copy()

@pytest.fixture(scope='function', name='oneseries_executor')
def __oneseries_executor():
    """provide a oneseries base"""
    return __ONESERIES_EXECUTOR.copy()

@pytest.fixture(scope='function', name='oneseries_fio')
def __oneseries_fio():
    """provide a fio oneseries"""
    return __ONESERIES_FIO.copy()

@pytest.fixture(scope='function', name='oneseries_ib_read')
def __oneseries_ib_read():
    """provide a fio oneseries"""
    return __ONESERIES_IB_READ.copy()

@pytest.fixture(scope='function')
def benchmark_dummy(oneseries_dummy):
    """create a dummy Benchmark instance"""
    return lib.benchmark.Benchmark(oneseries_dummy)

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
