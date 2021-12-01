#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021, Intel Corporation
#

"""test_init.py -- lib.Report.__init__() tests"""

import json
import shutil
import pytest

# XXX required for an easy comparison of nested dictionaries
# from deepdiff import DeepDiff
from lib.figure import Figure
from lib.report.part import Part
from lib.report.base import Report
import lib.report.base

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
NUMBER_OF_PARTS = 3

class BenchMock:
    """a lib.Bench.Bench mock"""
    config = {}
    result_dir = RESULT_DIR
    figures = [Figure(FIGURE_DUMMY, RESULT_DIR)]
    parts = [PART_DUMMY]

LOADER_DUMMY = {}

class EnvMock:
    """a jinja2.Environment mock"""
    filters = {}
    loader = LOADER_DUMMY

ENV_DUMMY = EnvMock()
BENCH_DUMMY = BenchMock()
TEST_DATE = "January 2077"

VARS_DUMMY = {
    'input_file': '/path/to/report.json',
    'json': {
        'test_date': TEST_DATE,
        'configuration': {
            'common': {},
            'target': {
                'details': {}
            },
            'bios': {
                'settings': {},
                'excerpt': {}
            },
            'schematic': SCHEMATIC_PATH
        }
    }
}

VARS_DUMMY_OUT = {
    'test_date': TEST_DATE,
    'configuration': {
        'common': {
            'type': 'kvtable',
            lib.report.base.__TESTING_DATE: TEST_DATE
        },
        'target': {
            'details': {
                'type': 'kvtable',
                lib.report.base.__TESTING_DATE: TEST_DATE
            }
        },
        'bios': {
            'settings': {
                'type': 'kvtable',
                lib.report.base.__TESTING_DATE: TEST_DATE
            },
            'excerpt': {
                'type': 'kvtable',
                lib.report.base.__TESTING_DATE: TEST_DATE
            }
        },
        'schematic': SCHEMATIC_FILE,
    }
}

@pytest.mark.skip(reason="XXX")
def test_init(monkeypatch):
    """a report with a single dummy part"""
    def loads_mock(source):
        assert source == ""
        return {}
    def to_html_mock(_self):
        return HTML_DUMMY
    def part_init_mock(self, env, name):
        assert env == ENV_DUMMY
        assert env.loader == LOADER_DUMMY
        assert name in ["preamble", "footer", PART_DUMMY]
        self.name = name
    def process_level_mock(variables, common):
        # XXX assert DeepDiff(variables, VARS_DUMMY_OUT) == {}
        assert variables == VARS_DUMMY_OUT
        assert common == {}
    # XXX how to mock a setter?
    # def set_variables_mock(self, variables):
    #     if self.name in ["preamble", "footer"]:
    #         # XXX assert DeepDiff(variables, VARS_DUMMY_OUT) == {}
    #         assert variables == VARS_DUMMY_OUT
    #     else:
    #         assert 'figure' in variables
    #         assert 'file_dummy' in variables['figure']
    #         assert 'key_dummy' in variables['figure']['file_dummy']
    #         assert variables['figure']['file_dummy']['key_dummy'] == \
    #             Figure(FIGURE_DUMMY, RESULT_DIR)
    def copy_mock(src, dst):
        assert src == SCHEMATIC_PATH
        assert dst == RESULT_DIR
    monkeypatch.setattr(json, 'loads', loads_mock)
    monkeypatch.setattr(Figure, 'to_html', to_html_mock)
    monkeypatch.setattr(Part, '__init__', part_init_mock)
    monkeypatch.setattr(lib.report.variable, 'process_level',
            process_level_mock)
    # monkeypatch.setattr(Part, 'set_variables', set_variables_mock)
    monkeypatch.setattr(shutil, 'copy', copy_mock)
    report = Report(ENV_DUMMY, BENCH_DUMMY, VARS_DUMMY)
    assert report.env == ENV_DUMMY
    # XXX assert DeepDiff(report.variables, VARS_DUMMY_OUT) == {}
    assert report.variables == VARS_DUMMY_OUT
    assert report.result_dir == RESULT_DIR
    assert len(report.parts) == NUMBER_OF_PARTS
