#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021, Intel Corporation
#

"""test_runner.py -- lib.benchmark.runner.Runner tests"""

import pytest
from pytest_mock import MockerFixture

import re
import shutil

import lib.benchmark
import lib.benchmark.runner
import lib.benchmark.runner.ib_read
from lib.benchmark.base import Benchmark
from lib.remote_cmd import RemoteCmd

from lib.benchmark.runner.ib_read import IbReadRunner


def test_IbReadRunner_init(mocker: MockerFixture):
    """test proper initialization of IbReadRunner object with all mandatory param"""
 
    oneseries = {'tool': 'ib_read', 'mode': 'lat',
                'rw': 'read', 'filetype': 'malloc',
                'requirements' : {'direct_write_to_pmem': True}}
    config = {'server_ip':'server_ip'}
    mock_which = mocker.patch('shutil.which', return_value = '/tmp/ib_read_lat')
    remoteCmd = RemoteCmd(None, None, None, exit_status = 0)
    mock_remoteCmd = mocker.patch('lib.benchmark.runner.ib_read.RemoteCmd.run_sync', return_value = remoteCmd)

    benchmark = lib.benchmark.Benchmark(oneseries)
    runner = IbReadRunner(benchmark,config, 'idfile')
    mock_which.assert_called_once()
    mock_which.assert_called_with('ib_read_lat')
    mock_remoteCmd.assert_called_once()


def test_IbReadRunner_init_oneserises_no_tool():
    """failed initialization of IbReadRunner object - no tool param provided """
    oneseries = {}
    config = {'server_ip':'server_ip'}
    benchmark = lib.benchmark.Benchmark(oneseries)
    with pytest.raises(ValueError):
        runner = IbReadRunner(benchmark,config, 'idfile')

def test_IbReadRunner_init_oneserises_no_toolmode():
    """failed initialization of IbReadRunner object - no tool_mode param provided """
    oneseries = {'tool': 'ib_read'}
    config = {'server_ip':'server_ip'}
    benchmark = lib.benchmark.Benchmark(oneseries)

    with pytest.raises(ValueError):
        runner = IbReadRunner(benchmark,config, 'idfile')

def test_IbReadRunner_init_oneserises_no_mode(mocker: MockerFixture):
    """failed initialization of IbReadRunner object - no mode param provided """
    oneseries = {'tool': 'ib_read'}
    config = {'server_ip':'server_ip'}
    benchmark = lib.benchmark.Benchmark(oneseries)

    with pytest.raises(ValueError):
        runner = IbReadRunner(benchmark,config, 'idfile')

def test_IbReadRunner_init_oneserises_no_rw(mocker: MockerFixture):
    """failed initialization of IbReadRunner object - no mode rw provided """
    oneseries = {'tool': 'ib_read', 'mode' : 'lat'}
    config = {'server_ip':'server_ip'}
    benchmark = lib.benchmark.Benchmark(oneseries)

    with pytest.raises(ValueError):
        runner = IbReadRunner(benchmark,config, 'idfile')

def test_IbReadRunner_init_oneserises_no_filetype(mocker: MockerFixture):
    """failed initialization of IbReadRunner object - no mode rw provided """
    oneseries = {'tool': 'ib_read', 'mode' : 'lat', 'rw':'read', }
    config = {'server_ip':'server_ip'}
    benchmark = lib.benchmark.Benchmark(oneseries)

    with pytest.raises(ValueError):
        runner = IbReadRunner(benchmark,config, 'idfile')

@pytest.mark.skip()
def test_IbReadRunner_init_oneserises_no_requirements(mocker: MockerFixture):
    """failed initialization of IbReadRunner object - no requirements provided """
    oneseries = {'tool': 'ib_read', 'mode': 'lat', 'rw':'read', 'filetype': 'malloc'}
    config = {'server_ip':'server_ip'}
    benchmark = lib.benchmark.Benchmark(oneseries)

    with pytest.raises(ValueError):
        runner = IbReadRunner(benchmark,config, 'idfile')

def test_IbReadRunner_init_no_config():
    """failed initialization of IbReadRunner object - no config provided """
    oneseries = {'tool': 'ib_read', 'mode': 'lat', 
                'rw':'read',  'filetype': 'malloc',
                'requirements' : {}}
    benchmark = lib.benchmark.Benchmark(oneseries)

    with pytest.raises(AttributeError):
        runner = IbReadRunner(benchmark,None, 'idfile')

@pytest.mark.skip()
def test_IbReadRunner_init_config_no_fio_path():
    """failed initialization of IbReadRunner object -
        - no server_ip in config provided """
    oneseries = {'tool': 'ib_read', 'mode': 'lat', 
                'rw':'read',  'filetype': 'malloc'}
    config = {}
    benchmark = lib.benchmark.Benchmark(oneseries)

    #with pytest.raises(ValueError):
    runner = IbReadRunner(benchmark,config, 'idfile')

def test_IbReadRunner_init_config_no_server_ip(mocker: MockerFixture):
    """failed initialization of IbReadRunner object -
        - no server_ip in config provided """
    oneseries = {'tool': 'ib_read', 'mode': 'lat', 
                'rw':'read',  'filetype': 'malloc'}
    config = {}
    benchmark = lib.benchmark.Benchmark(oneseries)
    mock_which = mocker.patch('shutil.which', return_value = '/tmp/ib_read')
    
    with pytest.raises(ValueError) as excinfo:
        runner = IbReadRunner(benchmark,config, 'idfile')
    mock_which.assert_called_once()
    mock_which.assert_called_with('ib_read_lat')
    
