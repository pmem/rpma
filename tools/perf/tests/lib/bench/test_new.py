#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021, Intel Corporation
#

"""test_new.py -- lib.bench.Bench.new() tests"""

import pytest

import lib.bench
import lib.figure

from lib.bench import Bench
from lib.benchmark import Benchmark
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

FIGURES_DUMMY_INPUT = [
    {
        'input_file': '/path/to/figure/' + PART + '.json',
        'json': FIGURES_DUMMY
    }
]

FIGURES_DUMMY_FLAT = [
    {
        'flat_key_1': 'flat_value_1',
        'flat_key_2': 'flat_value_2'
    }
]

BENCHMARKS_UNIQ = 'dummy benchmarks uniq'
REQUIREMENTS_UNIQ = 'dummy requirements uniq'

RESULT_DIR = '/dummy/path'

def bench_instance(monkeypatch):
    """provide a ready to test lib.bench instance"""
    def flatten_mock(figures_in, result_dir):
        assert figures_in == FIGURES_DUMMY
        assert result_dir == RESULT_DIR
        return FIGURES_DUMMY_FLAT
    def benchmark_uniq_mock(figures):
        assert figures == FIGURES_DUMMY_FLAT
        return BENCHMARKS_UNIQ
    def requirements_uniq_mock(benchmarks):
        assert benchmarks == BENCHMARKS_UNIQ
        return REQUIREMENTS_UNIQ
    monkeypatch.setattr(lib.bench, 'flatten', flatten_mock)
    monkeypatch.setattr(Benchmark, 'uniq', benchmark_uniq_mock)
    monkeypatch.setattr(Requirement, 'uniq', requirements_uniq_mock)
    return Bench.new(CONFIG_DUMMY_INPUT, FIGURES_DUMMY_INPUT, RESULT_DIR)

def test_simple(monkeypatch):
    """a very simple test"""
    bench = bench_instance(monkeypatch)
    # compare list of parts with the expected list of parts
    assert len(set(bench.parts).intersection(PARTS)) == len(PARTS)
    assert bench.config == CONFIG_DUMMY
    assert bench.figures == FIGURES_DUMMY_FLAT
    assert bench.requirements == REQUIREMENTS_UNIQ
    assert bench.result_dir == RESULT_DIR

@pytest.mark.parametrize('attr', ['parts', 'config', 'figures', 'requirements',
                                  'result_dir'])
def test_properties(attr, monkeypatch):
    """test lib.bench properties setters"""
    bench = bench_instance(monkeypatch)
    # setting any of these values should end up with an exception
    with pytest.raises(AttributeError):
        # it is an equivalent of bench.attr = 0
        setattr(bench, attr, 0)
