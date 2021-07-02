#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021, Intel Corporation
#

#
# kvtable.py -- key-value HTML table generator (EXPERIMENTAL)
#

def lines2str(lines, sep = "\n"):
    if type(lines) is str:
        return lines
    if hasattr(lines, '__iter__'):
        # merge a list of lines into a single string
        return sep.join(lines)
    return str(lines)

def dict2kvtable(obj, env):
    """Generate an HTML table from a dictionary"""
    return "XXX table"
