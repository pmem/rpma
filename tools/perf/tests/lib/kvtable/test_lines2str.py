#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021, Intel Corporation
#

"""test_lines2str.py -- lib.kvtable.lines2str() tests"""

import lib.kvtable

def test_empty():
    """produce an empty string"""
    output = lib.kvtable.lines2str([])
    assert output == ""
