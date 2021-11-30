#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021, Intel Corporation
#

"""test_png_path.py -- lib.figure.Figure.png_path() tests"""

import pytest
from lib.figure import Figure

DATA = {
    'output': {
        'title': 'title',
        'file': 'file',
        'x' : 'x',
        'y' : 'y',
        'key' : 'key'
    },
    'series': [
        {
            'label': 'label_1',
            'points' : [[0, 3], [1, 4], [2, 5]],
        }
    ]
}

RESULT_DIR = '/dummy/path'
RELATIVE_PATH = "./file_key.png"

def test_png_path_basic():
    """basic lib.figure.Figure.html_data_table() test"""
    assert Figure(DATA, RESULT_DIR)._Figure__png_path() == RELATIVE_PATH
