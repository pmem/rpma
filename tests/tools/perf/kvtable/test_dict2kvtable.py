#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021, Intel Corporation
#

"""test_dict2kvtable.py -- lib.kvtable.dict2kvtable() tests"""

import lib.kvtable

HEADER = '<tr><th>Item</th><th>Description</th></tr></thead><tbody>'
START = '<table><thead>'
STOP = '</tbody></table>'

EMPTY = START + HEADER + STOP
SIMPLE_RANGE = 10
SIMPLE = START + HEADER \
    + "".join([
        '<tr><td>{}</td><td><pre>{}</pre></td></tr>'.format(k, k + 1)
            for k in range(SIMPLE_RANGE)]) \
    + STOP

EMPTY_KVTABLE = {"type": "kvtable"}

def test_empty_empty():
    """produce an empty table"""
    output = lib.kvtable.dict2kvtable(EMPTY_KVTABLE, {})
    assert(output == EMPTY)

def test_simple():
    """produce a simple table n -> (n + 1)"""
    kvt = {str(k): str(k + 1) for k in range(SIMPLE_RANGE)}
    kvt["type"] = "kvtable"
    output = lib.kvtable.dict2kvtable(kvt, {})
    assert(output == SIMPLE)
