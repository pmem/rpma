#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021, Intel Corporation
#

"""test_lines2str.py -- lib.report.utils.lines2str() tests"""

import lib.report.utils
import pytest

DUMMY_STR = "dummy"

DUMMY_LIST = [
    "a",
    "bb",
    "ccc"
]

DUMMY_LIST_STR_DEFAULT = "a\nbb\nccc"

CUSTOM_SEP = "def"

DUMMY_LIST_STR_CUSTOM = "adefbbdefccc"

@pytest.mark.parametrize("arg,expected", [
    (DUMMY_STR, DUMMY_STR),
    (123, "123"),
    (DUMMY_LIST, DUMMY_LIST_STR_DEFAULT)])
def test_sep_default(arg, expected):
    """test lines2str() with the default separator value"""
    output = lib.report.utils.lines2str(arg)
    assert output == expected

def test_sep_custom():
    """test lines2str() with a custom separator value"""
    output = lib.report.utils.lines2str(DUMMY_LIST, CUSTOM_SEP)
    assert output == DUMMY_LIST_STR_CUSTOM
