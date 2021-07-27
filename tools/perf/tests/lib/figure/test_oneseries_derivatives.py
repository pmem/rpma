#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021, Intel Corporation
#

"""test_oneseries_derivatives.py -- lib.Figure.oneseries_derivatives() tests"""

from lib.Figure import Figure
import pytest

@pytest.mark.parametrize('rw, rw_order', [('randread', 'rand'), ('randwrite', 'rand'), ('read', 'seq'), ('write', 'seq')])
def test_oneseries_derivatives(rw, rw_order):
    """basic test of oneseries_derivatives()"""
    output = Figure.oneseries_derivatives({'rw' : rw})
    assert output['rw_order'] == rw_order