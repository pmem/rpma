#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021, Intel Corporation
#

#
# kvtable.py -- key-value HTML table generator (EXPERIMENTAL)
#

def lines2str(lines, sep = "\n"):
    """Merge a list of lines into a single string"""
    if isinstance(lines, str):
        return lines
    if hasattr(lines, '__iter__'):
        return sep.join(lines)
    return str(lines)

def dict2kvtable(obj, env):
    """Generate an HTML table from a dictionary"""
    # header
    html = "<table><tr><th>Item</th><th>Description</th></tr>"

    # rows
    for item, desc in obj.items():
        if item == "type":
            continue
        # replace all 'var' with 'txt' from env in all lines of 'desc'
        for line in desc:
            for var, txt in env.items():
                line.replace("{" + var + "}", txt)
        html += "<tr><td>" + str(item) + "</td><td>" + lines2str(desc) + "</td></tr>"

    # end the table
    html += "</table>"

    return html
