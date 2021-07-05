#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021, Intel Corporation
#

"""test_report_bench.py -- report_bench.py tests"""

import pytest
import report_bench

def test_no_args():
    """no arguments provided"""
    with pytest.raises(SystemExit):
        report_bench.main()
