#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021, Intel Corporation
#

"""test_set_id.py -- lib.Benchmark.set_id() and .get_id() tests"""

import pytest

from tests.lib.benchmark.conftest import DICT_DUMMY
from lib.Benchmark import Benchmark

ID_DUMMY = 80337

@pytest.mark.parametrize('from_figure', [True, False])
@pytest.mark.parametrize('identifier', [None, ID_DUMMY])
def test_get_initial_id(from_figure, identifier):
    """test an initial value of id"""
    oneseries_input = DICT_DUMMY.copy()
    if identifier is not None:
        oneseries_input['id'] = identifier
    benchmark = Benchmark(oneseries_input, from_figure=from_figure)
    # check the initial id value
    if identifier is None:
        assert benchmark.get_id() is None
    else:
        assert benchmark.get_id() == ID_DUMMY

@pytest.mark.parametrize('from_figure', [True, False])
def test_set_id_get_id(from_figure):
    """set a new id value and verify it was set properly"""
    oneseries_input = DICT_DUMMY.copy()
    benchmark = Benchmark(oneseries_input, from_figure=from_figure)
    # set a new id value
    benchmark.set_id(ID_DUMMY)
    # it is set correctly for the object
    assert benchmark.get_id() == ID_DUMMY
    # it was propagated up to the original series if it comes from a figure
    if from_figure:
        assert oneseries_input['id'] == ID_DUMMY
