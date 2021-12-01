#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021, Intel Corporation
#

"""test_init.py -- lib.Part.__init__() tests"""

import copy
import json
import lib.report.utils
import pytest

from lib.report.part import Part

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

    loader = LoaderMock()

def object_cmp(obj1, obj2):
    """compare objects by stringification"""
    dump1 = json.dumps(obj1, sort_keys=True)
    dump2 = json.dumps(obj2, sort_keys=True)
    return dump1 == dump2

@pytest.mark.parametrize('input_json,expected_constants',
    [({}, {}), ({'common': {}}, {})])
def test_empty(input_json, expected_constants, monkeypatch):
    """provide an (almost) empty input"""
    def loads_mock(arg):
        assert arg == SOURCE_DUMMY
        return input_json
    monkeypatch.setattr(json, 'loads', loads_mock)
    part = Part(EnvMock(), NAME_DUMMY)
    assert object_cmp(part.constants, expected_constants)

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
    STRING_DUMMY_2,
    STRING_DUMMY_1,
    STRING_DUMMY_1,
    STRING_DUMMY_1
]

COMMON_DUMMY_WITH_LIST = {
    VAR_DUMMY_3: STR_LIST_DUMMY_3
}

STRING_DUMMY_3_CONCAT = '\n'.join(STR_LIST_DUMMY_3)

COMMON_DUMMY_WITH_LIST_PROCESSED = {
    VAR_DUMMY_3: STRING_DUMMY_3_CONCAT
}

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

@pytest.mark.parametrize('input_json,expected_constants', [
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
def test_nested_misc(input_json, expected_constants, monkeypatch):
    """processing miscellaneous nested dictionaries"""
    def loads_mock(arg):
        assert arg == SOURCE_DUMMY
        return input_json
    monkeypatch.setattr(json, 'loads', loads_mock)
    part = Part(EnvMock(), NAME_DUMMY)
    assert object_cmp(part.constants, expected_constants)

@pytest.mark.parametrize('input_json,expected_constants', [
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
def test_nested_lines_misc(input_json, expected_constants, monkeypatch):
    """processing miscellaneous nested dictionaries with lists of strings"""
    def loads_mock(arg):
        assert arg == SOURCE_DUMMY
        return input_json
    def lines2str_mock(_lines):
        # XXX validate _lines
        return STRING_DUMMY_3_CONCAT
    monkeypatch.setattr(json, 'loads', loads_mock)
    # A function when imported becomes a part of the module. So, to mock
    # lib.kvtable.lines2str, the mock has to point where the function was
    # imported instead of the source module.
    monkeypatch.setattr(lib.report.utils, 'lines2str', lines2str_mock)
    part = Part(EnvMock(), NAME_DUMMY)
    assert object_cmp(part.constants, expected_constants)

TYPE_INVALID = {
    **JSON_NESTED,
    'type': 'invalid'
}

def test_type_invalid(monkeypatch):
    """test an object of an unknown 'type'"""
    def loads_mock(arg):
        assert arg == SOURCE_DUMMY
        return json_nested(None, TYPE_INVALID)
    monkeypatch.setattr(json, 'loads', loads_mock)
    with pytest.raises(ValueError):
        _ = Part(EnvMock(), NAME_DUMMY)

KVTABLE_DUMMY = {
    'type': 'kvtable',
    'zfat': 'bqfjoffgfi',
    'iwmr': 'bgegxaaqgv',
    'sklq': 'kyqdwmlqsp',
    'uhwc': 'dkwehngxav'
}

KVTABLE_OUTPUT_DUMMY = '<dummy>invalid</dummy>'

@pytest.mark.parametrize('input_json,expected_env,expected_constants', [
    # a simple kvtable nested inside a dictionary (an empty common)
    (json_nested(None, KVTABLE_DUMMY),
        {},
        json_nested(None, KVTABLE_OUTPUT_DUMMY)),
    # a simple kvtable nested inside a dictionary (a nonempty common)
    (json_nested(COMMON_DUMMY, KVTABLE_DUMMY),
        COMMON_DUMMY,
        json_nested(None, KVTABLE_OUTPUT_DUMMY)),
    # a simple kvtable nested inside a dictionary
    # (a nonempty common with list of lines)
    (json_nested(COMMON_DUMMY_WITH_LIST, KVTABLE_DUMMY),
        COMMON_DUMMY_WITH_LIST_PROCESSED,
        json_nested(None, KVTABLE_OUTPUT_DUMMY)),
    ])
def test_nested_kvtable_misc(input_json, expected_env, expected_constants,
        monkeypatch):
    """processing miscellaneous nested dictionaries with kvtables"""
    def loads_mock(arg):
        assert arg == SOURCE_DUMMY
        return input_json
    def lines2str_mock(_lines):
        # XXX validate _lines?
        return STRING_DUMMY_3_CONCAT
    def dict2kvtable_mock(obj, env):
        assert object_cmp(obj, KVTABLE_DUMMY)
        assert object_cmp(env, expected_env)
        return KVTABLE_OUTPUT_DUMMY
    monkeypatch.setattr(json, 'loads', loads_mock)
    # A function when imported becomes a part of the module.
    monkeypatch.setattr(lib.report.utils, 'lines2str', lines2str_mock)
    monkeypatch.setattr(lib.report.utils, 'dict2kvtable', dict2kvtable_mock)
    part = Part(EnvMock(), NAME_DUMMY)
    assert object_cmp(part.constants, expected_constants)
