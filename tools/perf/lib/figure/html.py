#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021-2022, Intel Corporation
#

#
# html.py
#

"""figure HTML tools (EXPERIMENTAL)"""

from ..common import escape

def __points_to_dict(points):
    """transform list of [x, y] into a dict() where {x: y}"""
    return {p[0]: p[1] for p in points}

def __get_xcommon(results):
    """generate the ordered list of common x-values"""
    xlist = [p[0]
             for oneseries in results
             for p in oneseries['points']]
    return sorted(list(set(xlist)))

def data_table(results: list, compare: bool = False) -> str:
    """combine results as HTML table

    Args:
        results: a list of results. Please see `lib.figure.base.Figure.results`.

        compare: when we generate a table for comparison, we do not need
                 the escape characters
    Returns:
        A str containing a HTML table combining all the `results`.
    """
    xcommon = __get_xcommon(results)
    # header
    html = '<table class="data"><thead><tr><th></th><th>{}</th></tr>' \
        '</thead><tbody>'. \
        format('</th><th>'.join([str(x) for x in xcommon]))
    # rows
    for oneseries in results:
        # Since the output is processed as markdown,
        # special characters have to be escaped.
        if compare:
            html += "<tr><td>" + oneseries['label'] + "</td>"
        else:
            html += "<tr><td>" + escape(oneseries['label']) + "</td>"
        points = __points_to_dict(oneseries['points'])
        points = {k: '{0:.2f}'.format(v) for k, v in points.items()}
        for xarg in xcommon:
            html += '<td>{}</td>'.format(points.get(xarg, '-'))
        html += "</tr>"

    # end the table
    html += "</tbody></table>"
    return html
