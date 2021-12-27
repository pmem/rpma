#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021, Intel Corporation
#

"""test_fiorunner_init.py -- lib.benchmark.runner.FioRunner init tests"""

import pytest
from pytest_mock import MockerFixture

import re
import shutil

import lib.benchmark
import lib.benchmark.runner
from lib.benchmark.base import Benchmark
from lib.remote_cmd import RemoteCmd

from lib.benchmark.runner.fio import FioRunner

def test_FioRunner_init(mocker: MockerFixture):
    """test proper initialization of FioRunner object with all mandatory param"""
 
    oneseries = {'tool': 'fio', 'mode': 'lat', 'tool_mode': 'apm',
                'rw': 'readwrite', 'busy_wait_polling': True, 
                'filetype': 'malloc',
                'requirements' : {'direct_write_to_pmem': True}}
    config = {'server_ip':'server_ip', 'FIO_PATH' : '/tmp'}
    mock_which = mocker.patch('shutil.which', return_value = '/tmp/fio')
    remoteCmd = RemoteCmd(None, None, None, exit_status = 0)
    mock_remoteCmd = mocker.patch('lib.benchmark.runner.fio.RemoteCmd.run_sync', return_value = remoteCmd)

    benchmark = lib.benchmark.Benchmark(oneseries)
    runner = FioRunner(benchmark,config, 'idfile')

    mock_which.assert_called_once()
    mock_which.assert_called_with('/tmp/fio')
    mock_remoteCmd.assert_called_once()


def test_FioRunner_init_oneserises_no_tool(mocker: MockerFixture):
    """failed initialization of FioRunner object - no tool param provided """
    oneseries = {}
    #'mode': 'lat', 'tool_mode': 'apm',
    #            'rw': 'readwrite', 'busy_wait_polling': True, 
    #            'filetype': 'malloc',
    #            'requirements' : {'direct_write_to_pmem': True}}
    config = {'server_ip':'server_ip', 'FIO_PATH' : '/tmp'}
    benchmark = lib.benchmark.Benchmark(oneseries)

    with pytest.raises(ValueError):
        runner = FioRunner(benchmark,config, 'idfile')

def test_FioRunner_init_oneserises_no_toolmode(mocker: MockerFixture):
    """failed initialization of FioRunner object - no tool_mode param provided """
    oneseries = {'tool': 'fio'}
    config = {'server_ip':'server_ip', 'FIO_PATH' : '/tmp'}
    benchmark = lib.benchmark.Benchmark(oneseries)

    with pytest.raises(ValueError):
        runner = FioRunner(benchmark,config, 'idfile')

def test_FioRunner_init_oneserises_no_mode(mocker: MockerFixture):
    """failed initialization of FioRunner object - no mode param provided """
    oneseries = {'tool': 'fio', 'tool_mode': 'apm'}
    config = {'server_ip':'server_ip', 'FIO_PATH' : '/tmp'}
    benchmark = lib.benchmark.Benchmark(oneseries)

    with pytest.raises(ValueError):
        runner = FioRunner(benchmark,config, 'idfile')

def test_FioRunner_init_oneserises_no_rw(mocker: MockerFixture):
    """failed initialization of FioRunner object - no mode rw provided """
    oneseries = {'tool': 'fio', 'tool_mode': 'apm', 'mode' : 'lat'}
    config = {'server_ip':'server_ip', 'FIO_PATH' : '/tmp'}
    benchmark = lib.benchmark.Benchmark(oneseries)

    with pytest.raises(ValueError):
        runner = FioRunner(benchmark,config, 'idfile')

def test_FioRunner_init_oneserises_no_filetype(mocker: MockerFixture):
    """failed initialization of FioRunner object - no mode rw provided """
    oneseries = {'tool': 'fio', 'tool_mode': 'apm', 'mode' : 'lat'}
    config = {'server_ip':'server_ip', 'FIO_PATH' : '/tmp'}
    benchmark = lib.benchmark.Benchmark(oneseries)

    with pytest.raises(ValueError):
        runner = FioRunner(benchmark,config, 'idfile')

def test_FioRunner_init_oneserises_no_requirements(mocker: MockerFixture):
    """failed initialization of FioRunner object - no requirements provided """
    oneseries = {'tool': 'fio', 'tool_mode': 'apm', 'mode': 'lat', 
                'rw':'readwrite', 'filetype': 'malloc'}
    config = {'server_ip':'server_ip', 'FIO_PATH' : '/tmp'}
    benchmark = lib.benchmark.Benchmark(oneseries)

    with pytest.raises(ValueError):
        runner = FioRunner(benchmark,config, 'idfile')

def test_FioRunner_init_oneserises_no_requirements_direct_write_to_pmem(mocker: MockerFixture):
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

def test_FioRunner_init_config_no_server_ip(mocker: MockerFixture):
    """failed initialization of FioRunner object -
        - no server_ip in config provided """
    oneseries = {'tool': 'fio', 'tool_mode': 'apm', 'mode': 'lat', 
                'rw':'readwrite',  'filetype': 'malloc',
                'requirements' : {'direct_write_to_pmem': True}}
    config = {'FIO_PATH' : '/tmp'}
    benchmark = lib.benchmark.Benchmark(oneseries)
    mock_which = mocker.patch('shutil.which', return_value = '/tmp/fio')

    with pytest.raises(ValueError) as excinfo:
        runner = FioRunner(benchmark,config, 'idfile')
    mock_which.assert_called_once()
    mock_which.assert_called_with('/tmp/fio')
    
