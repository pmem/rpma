#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021, Intel Corporation
#

"""test_init.py -- lib.Report.__init__() tests"""

import json
import shutil

# XXX required for an easy comparison of nested dictionaries
# from deepdiff import DeepDiff
from lib.Figure import Figure
from lib.Part import Part
from lib.Report import Report

FILE_DUMMY = "file_dummy"
KEY_DUMMY = "key_dummy"
HTML_DUMMY = "<html></html>"

FIGURE_DUMMY = {
    'output': {
        'title': 'Title dummy',
        'file': FILE_DUMMY,
        'x': 'x_dummy',
        'y': 'y_dummy',
        'key': KEY_DUMMY,
    },
    'series': []
}

RESULT_DIR = '/dummy/path'
PART_DUMMY = "dummy"
SCHEMATIC_FILE = 'Figure.png'
SCHEMATIC_PATH = 'path/' + SCHEMATIC_FILE

class BenchMock:
    """a lib.Bench.Bench mock"""
    result_dir = RESULT_DIR
    figures = [Figure(FIGURE_DUMMY)]
    parts = [PART_DUMMY]

class EnvMock:
    """a jinja2.Environment mock"""
    filters = {}

ENV_DUMMY = EnvMock()
LOADER_DUMMY = {}
BENCH_DUMMY = BenchMock()

VARS_DUMMY = {
    'input_file': '/path/to/report.json',
    'json': {
        'configuration': {
            'common': {},
            'target': {},
            'bios': {
                'settings': {},
                'excerpt': {}
            },
            'schematic': SCHEMATIC_PATH
        }
    }
}

VARS_DUMMY_OUT = {
    'configuration': {
        'common': {
            'type': 'kvtable'
        },
        'target': {
            'type': 'kvtable'
        },
        'bios': {
            'settings': {
                'type': 'kvtable'
            },
            'excerpt': {
                'type': 'kvtable'
            }
        },
        'schematic': SCHEMATIC_FILE,
    }
}

def test_init(monkeypatch):
    """a report with a single dummy part"""
    def loads_mock(source):
        assert source == ""
        return {}
    def to_html_mock(_self):
        return HTML_DUMMY
    def part_init_mock(self, loader, env, name):
        assert loader == LOADER_DUMMY
        assert env == ENV_DUMMY
        assert name in ["preamble", PART_DUMMY]
        self.name = name
    def process_variables_level_mock(_self, variables, common):
        # XXX assert DeepDiff(variables, VARS_DUMMY_OUT) == {}
        assert variables == VARS_DUMMY_OUT
        assert common == {}
    def set_variables_mock(self, variables):
        if self.name == "preamble":
            # XXX assert DeepDiff(variables, VARS_DUMMY_OUT) == {}
            assert variables == VARS_DUMMY_OUT
        else:
            assert 'figure' in variables
            assert 'file_dummy' in variables['figure']
            assert 'key_dummy' in variables['figure']['file_dummy']
            assert variables['figure']['file_dummy']['key_dummy'] == \
                Figure(FIGURE_DUMMY)
    def copy_mock(src, dst):
        assert src == SCHEMATIC_PATH
        assert dst == RESULT_DIR
    monkeypatch.setattr(json, 'loads', loads_mock)
    monkeypatch.setattr(Figure, 'to_html', to_html_mock)
    monkeypatch.setattr(Part, '__init__', part_init_mock)
    monkeypatch.setattr(Part, 'process_variables_level',
            process_variables_level_mock)
    monkeypatch.setattr(Part, 'set_variables', set_variables_mock)
    monkeypatch.setattr(shutil, 'copy', copy_mock)
    report = Report(LOADER_DUMMY, ENV_DUMMY, BENCH_DUMMY, VARS_DUMMY)
    assert report.env == ENV_DUMMY
    # XXX assert DeepDiff(report.variables, VARS_DUMMY_OUT) == {}
    assert report.variables == VARS_DUMMY_OUT
    assert report.result_dir == RESULT_DIR
    assert len(report.parts) == 2
