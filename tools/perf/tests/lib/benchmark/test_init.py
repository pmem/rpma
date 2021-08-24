#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021, Intel Corporation
#

"""test_init.py -- lib.Benchmark.__init__() tests"""

import pytest

from tests.lib.benchmark.conftest import DICT_DUMMY, KEY_DUMMY, VALUE_DUMMY
from lib.Benchmark import Benchmark, ENCODE

@pytest.mark.parametrize('oneseries', [{}, DICT_DUMMY])
@pytest.mark.parametrize('requirements', [None, DICT_DUMMY])
@pytest.mark.parametrize('from_figure', [None, True, False])
def test_simple(oneseries, requirements, from_figure):
    """a simple benchmark object"""
    oneseries_input = oneseries.copy()
    oneseries_output = oneseries.copy()
    if requirements is None:
        # if not requirements are provided they will turn out to be empty
        requirements = {}
    else:
        oneseries_input['requirements'] = requirements
    oneseries_output['done'] = False
    # build the named arguments dictionary
    kwargs = {}
    if from_figure is not None:
        kwargs['from_figure'] = from_figure
        if from_figure:
            # these keys should be removed
            oneseries_input['label'] = VALUE_DUMMY
            oneseries_input['rw_dir'] = VALUE_DUMMY
    benchmark = Benchmark(oneseries_input, **kwargs)
    if from_figure is True:
        # When from_figure the input should be copied
        # so it can be changed later on.
        oneseries_input[KEY_DUMMY] = VALUE_DUMMY
    assert str(benchmark) == ENCODE(oneseries_output)
    assert benchmark.get_requirements() == requirements

@pytest.mark.parametrize('readwrite', ['rw', 'randrw'])
@pytest.mark.parametrize('rw_dir', ['read', 'write'])
def test_rw(readwrite, rw_dir):
    """a rw_dir is required when *rw"""
    oneseries_input = DICT_DUMMY.copy()
    oneseries_output = DICT_DUMMY.copy()
    oneseries_input['rw'] = readwrite
    oneseries_output['rw'] = readwrite
    oneseries_input['rw_dir'] = rw_dir
    oneseries_output['done'] = False
    benchmark = Benchmark(oneseries_input, from_figure=True)
    assert str(benchmark) == ENCODE(oneseries_output)

@pytest.mark.parametrize('readwrite', ['rw', 'randrw'])
def test_rw_no_rw_dir(readwrite):
    """a rw_dir is required when *rw"""
    oneseries_input = DICT_DUMMY.copy()
    oneseries_input['rw'] = readwrite
    with pytest.raises(SyntaxError):
        _ = Benchmark(oneseries_input, from_figure=True)
