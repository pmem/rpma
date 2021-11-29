#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021, Intel Corporation
#

"""test_derivatives.py -- lib.figure.flat.__derivatives() tests"""

import lib.figure.flat
import pytest

@pytest.mark.parametrize('rw, rw_order', [('randread', 'rand'), ('randwrite', 'rand'), ('read', 'seq'), ('write', 'seq')])
def test_rw_order(rw, rw_order):
    """generating rw_order from rw"""
    output = lib.figure.flat.__derivatives({'rw' : rw})
    assert output['rw_order'] == rw_order

DUMMY_STR = 'dummy'
DUMMY_STR_KEY = 'key'

@pytest.mark.parametrize('input_key,output_key', [('x', 'x_key'),
    ('y', 'y_key')])
def test_xy_key(input_key, output_key, monkeypatch):
    """generating x_key and y_key from x and y"""
    def str2key_mock(arg):
        assert arg == DUMMY_STR
        return DUMMY_STR_KEY
    monkeypatch.setattr(lib.figure.flat, '__str2key', str2key_mock)
    output = lib.figure.flat.__derivatives({input_key: DUMMY_STR})
    assert output[output_key] == DUMMY_STR_KEY
