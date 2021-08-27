#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021, Intel Corporation
#

"""test_carry_on.py -- lib.bench.Bench.carry_on() tests"""

import os

from lib.bench import Bench

DUMMY_INPUT_PATH = 'dummy'

BENCH_IN = {
    'input_file': DUMMY_INPUT_PATH,
    'json': {
        'figures': {},
        'requirements': {},
        'config': {},
        'parts': {}
    }
}

REALPATH_OUT = 'realpath_out_dummy'
SPLIT_OUT_1 = 'split_out_1_dummy'

def test_result_dir(monkeypatch):
    """
    assert the result dir_is calculated from /input/path/bench.json using
    os.path.realpath() and os.path.split()
    """
    def realpath_mock(path_in):
        assert path_in == DUMMY_INPUT_PATH
        return REALPATH_OUT
    def split_mock(path_in):
        assert path_in == REALPATH_OUT
        return SPLIT_OUT_1, None
    monkeypatch.setattr(os.path, 'realpath', realpath_mock)
    monkeypatch.setattr(os.path, 'split', split_mock)
    bench_out = Bench.carry_on(BENCH_IN)
    assert bench_out.result_dir == SPLIT_OUT_1
