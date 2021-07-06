#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021, Intel Corporation
#

"""test_init.py -- lib.Part.__init__() tests"""

import copy
import json
import lib.kvtable
import pytest

from lib.Part import Part

NAME_DUMMY = 'dummy'
TEMPLATE_DUMMY = 'dummy'
SOURCE_DUMMY = 'dummy'

class LoaderMock:
    """a simple jinja2.BaseLoader subclass mock"""

    def get_source(self, _env, template):
        """a simple pass-through mock with basic check"""
        assert isinstance(self, LoaderMock), "silence no-self-use"
        assert isinstance(_env, EnvMock)
        return (SOURCE_DUMMY, template, True)

class EnvMock:
    """a simple jinja2.Environment class mock"""

    def get_template(self, name):
        """a simple pass-through mock with basic check"""
        assert isinstance(self, EnvMock), "silence no-self-use"
        assert name == 'part_' + NAME_DUMMY + '.md'
        return TEMPLATE_DUMMY

def dictcmp(dict1, dict2):
    """compare dictionaries by stringification"""
    dump1 = json.dumps(dict1, sort_keys=True)
    dump2 = json.dumps(dict2, sort_keys=True)
    return dump1 == dump2

@pytest.mark.parametrize('input_json,expected_variables',
    [({}, {}), ({'common': {}}, {})])
def test_empty(input_json, expected_variables, monkeypatch):
    """provide an (almost) empty input"""
    def loads_mock(arg):
        assert arg == SOURCE_DUMMY
        return input_json
    monkeypatch.setattr(json, 'loads', loads_mock)
    part = Part(LoaderMock(), EnvMock(), NAME_DUMMY)
    assert dictcmp(part.variables, expected_variables)

JSON_NESTED = {
    'a': {
        'b': {
            'c': {
                'd': {
                    'e': 0
                }
            }
        },
    },
    'f': {
        'g': {
            'h': {
                'i': 0
            }
        }
    }
}

STRING_DUMMY_1 = 'jmvfzrvvul'
STRING_DUMMY_2 = 'uupufjytlb'
VAR_DUMMY_1 = 'tmjt'
VAR_DUMMY_2 = 'lwhi'
VAR_DUMMY_3 = 'skld'

COMMON_DUMMY = {
    VAR_DUMMY_1: STRING_DUMMY_1,
    VAR_DUMMY_2: STRING_DUMMY_2
}

STR_LIST_DUMMY_3 = [
    STRING_DUMMY_1,
    STRING_DUMMY_1,
    STRING_DUMMY_1,
    STRING_DUMMY_1,
    STRING_DUMMY_1
]

COMMON_DUMMY_WITH_LIST = {
    VAR_DUMMY_3: STR_LIST_DUMMY_3
}

STRING_DUMMY_3_CONCAT = 'jlkfsdlkjrwklj'

def json_nested(common=None, inner_e=None, inner_i=None):
    """produce a deeply-nested dictionary
    Args:
        common (dict, optional): a output['common'] contents
        inner_e (dict, optional): a output['a']['b']['c']['d']['e'] contents
        inner_i (dict, optional): a output['f']['g']['h']['i'] contents
    Returns:
        a deeply nested dictionary
    """
    nested = copy.deepcopy(JSON_NESTED)
    if common is not None:
        nested['common'] = common
    if inner_e is not None:
        nested['a']['b']['c']['d']['e'] = inner_e
    if inner_i is not None:
        nested['f']['g']['h']['i'] = inner_i
    return nested

@pytest.mark.parametrize('input_json,expected_variables', [
    # a simple nested dictionary
    (json_nested(), json_nested()),
    # a simple nested dictionary with various strings inside
    (json_nested(None, STRING_DUMMY_1, STRING_DUMMY_2),
        json_nested(None, STRING_DUMMY_1, STRING_DUMMY_2)),
    # a single variable inside a nested dictionary
    (json_nested(COMMON_DUMMY, '{' + VAR_DUMMY_1 + '}'),
        json_nested(None, STRING_DUMMY_1)),
    # another variable inside a nested dictionary
    (json_nested(COMMON_DUMMY, '{' + VAR_DUMMY_2 + '}'),
        json_nested(None, STRING_DUMMY_2)),
    # two variables in two different places inside a nested dictionary
    (json_nested(
            COMMON_DUMMY, '{' + VAR_DUMMY_2 + '}', '{' + VAR_DUMMY_1 + '}'),
        json_nested(None, STRING_DUMMY_2, STRING_DUMMY_1)),
    # two variables in the same place inside a nested dictionary
    (json_nested(
            COMMON_DUMMY, '{' + VAR_DUMMY_2 + '}...{' + VAR_DUMMY_1 + '}'),
        json_nested(None, STRING_DUMMY_2 + '...' + STRING_DUMMY_1))
    ])
def test_nested_misc(input_json, expected_variables, monkeypatch):
    """processing miscellaneous nested dictionaries"""
    def loads_mock(arg):
        assert arg == SOURCE_DUMMY
        return input_json
    monkeypatch.setattr(json, 'loads', loads_mock)
    part = Part(LoaderMock(), EnvMock(), NAME_DUMMY)
    assert dictcmp(part.variables, expected_variables)

@pytest.mark.parametrize('input_json,expected_variables', [
    # a list of strings inside a nested dictionary
    (json_nested(None, STR_LIST_DUMMY_3),
        json_nested(None, STRING_DUMMY_3_CONCAT)),
    # a variable representing a list of strings inside a nested dictionary
    (json_nested(COMMON_DUMMY_WITH_LIST, '{' + VAR_DUMMY_3 + '}'),
        json_nested(None, STRING_DUMMY_3_CONCAT)),
    # a variable representing a list of strings and another list of strings
    # both at the same time inside a nested dictionary
    (json_nested(
            COMMON_DUMMY_WITH_LIST, '{' + VAR_DUMMY_3 + '}', STR_LIST_DUMMY_3),
        json_nested(None, STRING_DUMMY_3_CONCAT, STRING_DUMMY_3_CONCAT))
    ])
def test_nested_lines_misc(input_json, expected_variables, monkeypatch):
    """processing miscellaneous nested dictionaries with lists of strings"""
    def loads_mock(arg):
        assert arg == SOURCE_DUMMY
        return input_json
    def lines2str_mock(_lines):
        return STRING_DUMMY_3_CONCAT
    monkeypatch.setattr(json, 'loads', loads_mock)
    # A function when imported becomes a part of the module. So, to mock
    # lib.kvtable.lines2str, the mock has to point where the function was
    # imported instead of the source module.
    monkeypatch.setattr(lib.Part, 'lines2str', lines2str_mock)
    part = Part(LoaderMock(), EnvMock(), NAME_DUMMY)
    assert dictcmp(part.variables, expected_variables)

def test_nested_kvtable():
    """XXX"""
    pass
