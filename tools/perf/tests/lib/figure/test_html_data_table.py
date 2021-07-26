#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021, Intel Corporation
#

"""test_html_data_table.py.py -- lib.Figure.html_data_table() tests"""

import pytest
from lib.common import json_from_file
from lib.Figure import Figure

POINTS_1 = [[0, 3], [1, 4], [2, 5]]
POINTS_2 = [[0, 6], [1, 7], [2, 8]]

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
            'points' : POINTS_1,
        },
        {
            'label': 'label_2',
            'points' : POINTS_2,
        }
    ]
}

HTML = "<table><tr><th></th><th>0</th><th>1</th><th>2</th></tr><tr><td>label_1</td><td>3</td><td>4</td><td>5</td></tr><tr><td>label_2</td><td>6</td><td>7</td><td>8</td></tr></table>"

def test_html_data_table_basic():
    """ basic lib.Figure.html_data_table() test """
    assert Figure(DATA).html_data_table() == HTML
