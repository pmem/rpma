#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021, Intel Corporation
#

#
# variable.py
#

"""key-value HTML table generator (EXPERIMENTAL)"""

def lines2str(lines, sep="\n"):
    """Merge a list of lines into a single string

    Args:
        lines (list, str, other): a list of lines or a single object
        sep (str, optional): a separator

    Returns:
        str: a single string which is either a concatenated lines (using
            a custom or the default separator) or a str(lines) result
    """
    if isinstance(lines, str):
        return lines
    if hasattr(lines, '__iter__'):
        return sep.join(lines)
    return str(lines)

def dict2kvtable(obj, env):
    """Generate an HTML table from a dictionary"""
    # header
    html = """<table><thead><tr><th>Item</th><th>Description</th></tr></thead><tbody>"""

    # rows
    for item, desc in obj.items():
        if item == "type":
            continue
        # replace all 'var' with 'txt' from env in all lines of 'desc'
        for line in desc:
            for var, txt in env.items():
                line.replace("{" + var + "}", txt)
        html += "<tr><td>" + str(item) + "</td><td><pre>" + lines2str(desc) + \
            "</pre></td></tr>"

    # end the table
    html += "</tbody></table>"

    return html

def process_level(variables, common):
    """Process a level of template variables including:
    - concatenating multiline strings
    - replacing common variables
    - replacing kvtable definitions with their representation

    Args:
        variables (dict): a level of template variables
        common (dict): a map of common variables reused across the template
            variables
    """
    for key, value in variables.items():
        if isinstance(value, list):
            variables[key] = lines2str(value)
        elif isinstance(value, dict):
            if 'type' in value:
                if value["type"] == "kvtable":
                    variables[key] = dict2kvtable(value, common)
                else:
                    raise ValueError
            else:
                process_level(value, common)
        # replace common
        if isinstance(variables[key], str):
            variables[key] = variables[key].format(**common)
