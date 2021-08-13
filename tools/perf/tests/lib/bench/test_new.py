#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021, Intel Corporation
#

"""test_new.py -- lib.Bench.new() tests"""

from lib.Bench import Bench
from lib.Benchmark import Benchmark
from lib.Figure import Figure
from lib.Requirement import Requirement

DICT_DUMMY = {
    'dummy_key_1': 'dummy_value_1',
    'dummy_key_2': 'dummy_value_2'
}

CONFIG_DUMMY = DICT_DUMMY

CONFIG_DUMMY_INPUT = {
    'input_file': '/path/to/config.json',
    'json': CONFIG_DUMMY
}

PART = 'figure'

PARTS = [PART]

FIGURES_DUMMY = [DICT_DUMMY]

FIGURES_DUMMY_INPUT = {
    'input_file': '/path/to/figure/' + PART + '.json',
    'json': FIGURES_DUMMY
}

FIGURES_DUMMY_FLAT = [
    {
        'flat_key_1': 'flat_value_1',
        'flat_key_2': 'flat_value_2'
    }
]

BENCHMARKS_UNIQ = 'dummy benchmarks uniq'
REQUIREMENTS_UNIQ = 'dummy requirements uniq'

RESULT_DIR = '/dummy/path'

def test_simple(monkeypatch):
    """a very simple test"""
    def flatten_mock(figures_in):
        assert figures_in == FIGURES_DUMMY
        return FIGURES_DUMMY_FLAT
    def benchmark_uniq_mock(figures):
        assert figures == FIGURES_DUMMY_FLAT
        return BENCHMARKS_UNIQ
    def requirements_uniq_mock(benchmarks):
        assert benchmarks == BENCHMARKS_UNIQ
        return REQUIREMENTS_UNIQ
    monkeypatch.setattr(Figure, 'flatten', flatten_mock)
    monkeypatch.setattr(Benchmark, 'uniq', benchmark_uniq_mock)
    monkeypatch.setattr(Requirement, 'uniq', requirements_uniq_mock)
    bench = Bench.new(CONFIG_DUMMY_INPUT, FIGURES_DUMMY_INPUT, RESULT_DIR)
    # compare list of parts with the expected list of parts
    assert len(set(bench.parts).intersection(PARTS)) == len(PARTS)
    assert bench.config == CONFIG_DUMMY
    assert bench.figures == FIGURES_DUMMY_FLAT
    assert bench.requirements == REQUIREMENTS_UNIQ
    assert bench.result_dir == RESULT_DIR
