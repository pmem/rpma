#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021, Intel Corporation
#

"""test_report_create.py -- report_create.py tests"""

import pytest
import report_create

def test_no_args():
    """no arguments provided"""
    with pytest.raises(SystemExit):
        report_create.main()
