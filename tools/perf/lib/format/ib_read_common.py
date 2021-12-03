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

def grep_output(output : str, pattern : str) -> str:
    """grep for one line with results

    Args:
        output: a string to be grepped
        pattern: a pattern to look for

    Returns:
        one line with results
    """
    line = ''
    for line in output.splitlines():
        if re.search(pattern, line) and not re.search('[B]', line) and \
            not re.search('RKey', line):
            break
    return line

def line2csv(line : str) -> str:
    """convert the line with results to csv format

    Args:
        line: a line to be converted

    Returns:
        a line in csv format
    """
    line = re.sub(r'^[ ]*', '', line)
    line = re.sub(r'[ ]*$', '', line)
    line = re.sub(r'\s+', ',', line)
    return line
