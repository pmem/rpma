#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021, Intel Corporation
#

"""test_oneseries_derivatives.py -- lib.Figure.oneseries_derivatives() tests"""

from lib.Figure import Figure
import pytest

def test_oneseries_derivatives_randread():
    """test of oneseries_derivatives() for rw == randread"""
    oneseries = { 'rw' : 'randread' }
    output = Figure.oneseries_derivatives(oneseries)
    assert output['rw_order'] == 'rand'

def test_oneseries_derivatives_randwrite():
    """test of oneseries_derivatives() for rw == randwrite"""
    oneseries = { 'rw' : 'randwrite' }
    output = Figure.oneseries_derivatives(oneseries)
    assert output['rw_order'] == 'rand'

def test_oneseries_derivatives_read():
    """test of oneseries_derivatives() for rw == read"""
    oneseries = { 'rw' : 'read' }
    output = Figure.oneseries_derivatives(oneseries)
    assert output['rw_order'] == 'seq'

def test_oneseries_derivatives_write():
    """test of oneseries_derivatives() for rw == write"""
    oneseries = { 'rw' : 'write' }
    output = Figure.oneseries_derivatives(oneseries)
    assert output['rw_order'] == 'seq'
