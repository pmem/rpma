#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021, Intel Corporation
#

"""test_html_data_table.py -- lib.figure.Figure.html_data_table() tests"""

import lib.figure.html

KEY = 'key'

DATA = {
    'output': {
        'title': 'title',
        'file': 'file',
        'x' : 'x',
        'y' : 'y',
        'key' : KEY,
        'done': True
    },
    'series': []
}

RESULT_DIR = '/dummy/path'

SERIES_FILE = 'dummy/series/file'

SERIES = {
    'json': {
        KEY : {
            'common_params': {},
            'series': [
                {
                    'label': 'label_1',
                    'points' : [[0, 3], [1, 4], [2, 5]],
                },
                {
                    'label': 'label_2',
                    'points' : [[0, 6], [1, 7], [2, 8]],
                }
            ]
        }
    }
}

HTML = '<table class="data"><thead><tr><th></th><th>0</th><th>1</th><th>2</th></tr></thead><tbody><tr><td>label\\_1</td><td>3.00</td><td>4.00</td><td>5.00</td></tr><tr><td>label\\_2</td><td>6.00</td><td>7.00</td><td>8.00</td></tr></tbody></table>'

def test_html_data_table_basic(monkeypatch):
    """basic lib.figure.Figure.html_data_table() test"""
    def series_file_mock(_self, _):
        return SERIES_FILE
    def json_from_file_mock(series_file):
        assert series_file == SERIES_FILE
        return SERIES
    monkeypatch.setattr(lib.figure.Figure, '_Figure__series_file', series_file_mock)
    monkeypatch.setattr(lib.figure.base, 'json_from_file',
        json_from_file_mock)
    figure = lib.figure.Figure(DATA, RESULT_DIR)
    assert lib.figure.html.data_table(figure.results) == HTML
