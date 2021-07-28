#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021, Intel Corporation
#

"""test_str2key.py -- lib.common.str2key() tests"""

import lib.common
import pytest

@pytest.mark.parametrize('arg, exp',
    [('dummy', 'dummy'), ('d.u.m.m.y', 'dummy')])
def test_key(arg, exp):
    """a valid jinja2 template key has to be returned"""
    assert lib.common.str2key(arg) == exp
