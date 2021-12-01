#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021, Intel Corporation
#

"""test_dummy_run.py -- lib.benchmark.runner.Dummy.run() tests"""

import json
import random

import lib.benchmark

DUMMY_STR = 'dummy'
DUMMY_RANDOM = 138
VALID_KEYS = ['threads', 'iodepth', 'bs', 'ops', 'lat_min', 'lat_max',
    'lat_avg', 'lat_stdev', 'lat_pctl_99.0', 'lat_pctl_99.9',
    'lat_pctl_99.99', 'lat_pctl_99.999', 'bw_min', 'bw_max', 'bw_avg',
    'iops_min', 'iops_max', 'iops_avg', 'cpuload']

def test_dummy_results(benchmark_dummy, tmpdir, monkeypatch):
    """happy day scenario + validation of the generated output"""
    def dump_mock(obj, filepointer, indent=None):
        for point in obj:
            assert len(point.items()) == len(VALID_KEYS)
            for key, value in point.items():
                assert key in VALID_KEYS
                assert value == DUMMY_RANDOM
        filepointer.write(DUMMY_STR)
        assert indent is None or isinstance(indent, int)
    def randint_mock(_a, _b):
        return DUMMY_RANDOM
    monkeypatch.setattr(random, 'randint', randint_mock)
    monkeypatch.setattr(json, 'dump', dump_mock)
    output = lib.benchmark.get_result_path(str(tmpdir),
                                           benchmark_dummy.identifier)
    lib.benchmark.runner.Dummy.run(benchmark_dummy, None, output)
    # marking a benchmark as 'done' is done outside of the runner
    assert not benchmark_dummy.is_done()
    with open(output, mode='r', encoding='utf-8') as file:
        assert DUMMY_STR == file.read()
