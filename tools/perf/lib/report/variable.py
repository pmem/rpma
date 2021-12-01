#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021, Intel Corporation
#

#
# variable.py
#

"""key-value HTML table generator (EXPERIMENTAL)"""

from ..common import escape

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

__TESTING_DATE = 'Testing Date'

def __preprocess_config_table(table, variables):
    """set type and append the testing date to a configuration table"""
    # the only correct type is 'kvtable'
    table['type'] = 'kvtable'
    table[__TESTING_DATE] = variables['test_date']

def preprocess_vars(variables):
    """XXX"""
    input_file = variables['input_file']
    variables = variables['json']
    missing = None

    if 'authors' in variables:
        variables['authors'] = "\n".join(
            ['- ' + author for author in variables['authors']])

    if 'ref' in variables:
        variables['ref'] = escape(variables['ref'])

    # XXX type validation is missing
    if 'configuration' not in variables:
        missing = "['configuration']"
    else:
        if 'common' not in variables['configuration']:
            missing = "['configuration']['common']"
        elif 'target' not in variables['configuration']:
            missing = "['configuration']['target']"
        elif 'bios' not in variables['configuration']:
            missing = "['configuration']['bios']"
        else:
            if 'settings' not in variables['configuration']['bios']:
                missing = "['configuration']['bios']['settings']"
            elif 'excerpt' not in variables['configuration']['bios']:
                missing = "['configuration']['bios']['excerpt']"

    if missing is not None:
        raise SyntaxError("{} misses {} entry".format(input_file, missing))

    # escape markdown special characters
    if 'security' in variables['configuration']:
        variables['configuration']['security'] = \
            escape(variables['configuration']['security'])
    if 'description' in variables['configuration']:
        variables['configuration']['description'] = \
            escape(variables['configuration']['description'])

    for table in [variables['configuration']['common'],
                  variables['configuration']['target']['details'],
                  variables['configuration']['bios']['settings'],
                  variables['configuration']['bios']['excerpt']]:
        __preprocess_config_table(table, variables)
    return variables
