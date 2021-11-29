#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021, Intel Corporation
#

#
# html.py
#

"""XXX
"""

from ..common import escape

def __points_to_dict(points):
    """transform list of [x, y] into a dict() where {x: y}"""
    return {p[0]: p[1] for p in points}

def data_table(xcommon: list, series: list):
    """
    Create an HTML snippet string with a table containing the Figure data.
    """
    # header
    html = '<table class="data"><thead><tr><th></th><th>{}</th></tr>' \
        '</thead><tbody>'. \
        format('</th><th>'.join([str(x) for x in xcommon]))
    # rows
    for oneseries in series:
        # Since the output is processed as markdown,
        # special characters have to be escaped.
        html += "<tr><td>" + escape(oneseries['label']) + "</td>"
        points = __points_to_dict(oneseries['points'])
        points = {k: '{0:.2f}'.format(v) for k, v in points.items()}
        for xarg in xcommon:
            html += '<td>{}</td>'.format(points.get(xarg, '-'))
        html += "</tr>"

    # end the table
    html += "</tbody></table>"
    return html
