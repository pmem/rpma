#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021, Intel Corporation
#

"""test_uniq.py -- lib.Benchmark.uniq() tests"""

import lib.Benchmark

SERIES_1 = {
    'dummy_key_1': 'dummy_value_1'
}

SERIES_2 = {
    'dummy_key_2': 'dummy_value_2'
}

SERIES_3 = {
    'dummy_key_3': 'dummy_value_3'
}

class FigureMock:
    """a very simple lib.Figure.Figure replacement"""
    def __init__(self, series):
        self.series = series

FIGURES_DUMMY = [
    FigureMock([SERIES_1, SERIES_2]),
    FigureMock([SERIES_3])
]

UNIQ_DUMMY_OUTPUT = "dummy"

def test_simple(monkeypatch):
    """a simple benchmark object"""
    def uniq_mock(elems):
        for elem in elems:
            match = False
            for series in [SERIES_1, SERIES_2, SERIES_3]:
                if elem.oneseries == series:
                    match = True
                    break
            assert match, "an unexpected value found:\n{}".format(elem)
        return UNIQ_DUMMY_OUTPUT
    def benchmark_init_mock(self, oneseries, **kwargs):
        assert kwargs['from_figure']
        self.oneseries = oneseries
    monkeypatch.setattr(lib.Benchmark, 'uniq', uniq_mock)
    monkeypatch.setattr(lib.Benchmark.Benchmark, '__init__',
        benchmark_init_mock)
    output = lib.Benchmark.Benchmark.uniq(FIGURES_DUMMY)
    assert output == UNIQ_DUMMY_OUTPUT
