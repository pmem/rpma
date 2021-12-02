#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021, Intel Corporation
#

#
# utils.py
#

"""variable processing tools (EXPERIMENTAL)"""

from typing import Any, Union

def lines2str(lines: Union[list, str, Any], sep: str = "\n") -> str:
    """Merge a list of lines into a single string

    Args:
        lines: a list of lines or a single object
        sep: a separator

    Returns:
        A single string which is either a concatenated lines (using a custom
        or the default separator) or a `str(lines)` result.
    """
    if isinstance(lines, str):
        return lines
    if hasattr(lines, '__iter__'):
        return sep.join(lines)
    return str(lines)

def dict2kvtable(obj: dict, env: dict) -> str:
    """Generate an HTML table from a dictionary

    The proto-HTML table (`obj`) provides as `dict` as `item:description` pairs.
    - The `item` is rendered on the left-hand side of the table. The `item` is
      simple `str` and is passed to the output without any processing.
    - The `description` is rendered on the right-hand side of the table.
      It is a list of `str`. Additionally, it may have placeholders (`{var}`)
      that are replaced with the values provided by the `env` argument.

    Args:
        obj: a proto-HTML table
        env: a set of values to be replaced inside `description`s.

    Returns:
        An HTML string containing the generated table.
    """
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

def process_level(variables: dict, common: dict) -> None:
    """Process a level of template variables including:

    - concatenating multiline strings
    - replacing common variables
    - replacing kvtable definitions with their representation (`dict2kvtable()`)

    Args:
        variables: a level of template variables
        common: a map of common variables reused across the template variables
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
