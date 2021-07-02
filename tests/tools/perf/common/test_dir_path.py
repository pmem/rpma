#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021, Intel Corporation
#

"""test_dir_path.py -- lib.common.dir_path() tests"""

import os
import pytest

import lib.common

DUMMY_DIR = 'dummy'
DUMMY_ABS_DIR = '/abs/dummy'

@pytest.fixture
def arg_exists(monkeypatch):
    """configure the os.path.exists() mock for success"""
    def exists_mock(path):
        assert(path == DUMMY_DIR)
        return True
    monkeypatch.setattr(os.path, 'exists', exists_mock)

@pytest.mark.usefixtures("arg_exists")
def test_arg_not_isdir(monkeypatch):
    """arg is not a directory"""
    def is_dir_mock(path):
        assert(path == DUMMY_DIR)
        return False
    monkeypatch.setattr(os.path, 'isdir', is_dir_mock)
    with pytest.raises(NotADirectoryError):
        _ = lib.common.dir_path(DUMMY_DIR)

@pytest.mark.usefixtures("arg_exists")
def test_happy_day(monkeypatch):
    """arg exists and is a directory"""
    def isdir_mock(path):
        assert(path == DUMMY_DIR)
        return True
    def abspath_mock(path):
        assert(path == DUMMY_DIR)
        return DUMMY_ABS_DIR
    monkeypatch.setattr(os.path, 'isdir', isdir_mock)
    monkeypatch.setattr(os.path, 'abspath', abspath_mock)
    output = lib.common.dir_path(DUMMY_DIR)
    assert(output == DUMMY_ABS_DIR)
