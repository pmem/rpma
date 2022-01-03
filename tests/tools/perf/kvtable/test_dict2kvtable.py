#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021, Intel Corporation
#

"""test_dict2kvtable.py -- lib.report.utils.dict2kvtable() tests"""

import lib.report.utils

HEADER = '<tr><th>Item</th><th>Description</th></tr></thead><tbody>'
START = '<table><thead>'
STOP = '</tbody></table>'

EMPTY = START + HEADER + STOP
SIMPLE_RANGE = 10
KVT = {str(k): str(k + 1) for k in range(SIMPLE_RANGE)}
SIMPLE = START + HEADER \
    + "".join([
        '<tr><td>{}</td><td><pre>{}</pre></td></tr>'.format(k, KVT[k])
            for k in KVT.keys()]) \
    + STOP

EMPTY_KVTABLE = {"type": "kvtable"}

def test_empty_empty():
    """produce an empty table"""
    output = lib.report.utils.dict2kvtable(EMPTY_KVTABLE, {})
    assert(output == EMPTY)

def test_simple():
    """produce a simple table n -> (n + 1)"""
    KVT["type"] = "kvtable"
    output = lib.report.utils.dict2kvtable(KVT, {})
    assert(output == SIMPLE)
