#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021, Intel Corporation
#

"""test_dict2kvtable.py -- lib.kvtable.dict2kvtable() tests"""

import lib.kvtable

HEADER = '<tr><th>Item</th><th>Description</th></tr>'
START = '<table>'
STOP = '</table>'

EMPTY = START + HEADER + STOP
SIMPLE_RANGE = 10
SIMPLE = START + HEADER \
    + "".join([
        '<tr><td>{}</td><td>{}</td></tr>'.format(k, k + 1)
            for k in range(SIMPLE_RANGE)]) \
    + STOP

def test_empty_empty():
    """produce an empty table"""
    output = lib.kvtable.dict2kvtable({}, {})
    assert(output == EMPTY)

def test_simple():
    """produce a simple table n -> (n + 1)"""
    output = lib.kvtable.dict2kvtable(
        # XXX dict2kvtable does not accept keys or values which are not str!
        # XXX lines2str() when feeded with a single string instead of a list
        # is putting '\n' inside the string e.g. '10' -> '1\n0'
        {str(k): str(k + 1) for k in range(SIMPLE_RANGE)}, {})
    assert(output == SIMPLE)
