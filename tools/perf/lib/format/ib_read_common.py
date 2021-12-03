#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021, Intel Corporation
#

#
# ib_read_common.py
#

"""the format helpers (EXPERIMENTAL)"""

import re

def grep_output(output, pattern):
    """grep one line with results"""
    line = ''
    for line in output.splitlines():
        if re.search(pattern, line) and not re.search('[B]', line) and \
            not re.search('RKey', line):
            break
    return line

def line2csv(line):
    """convert the line with results to csv format"""
    line = re.sub(r'^[ ]*', '', line)
    line = re.sub(r'[ ]*$', '', line)
    line = re.sub(r'\s+', ',', line)
    return line
