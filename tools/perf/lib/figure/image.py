#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021, Intel Corporation
#

#
# image.py
#

"""figure imaging tools (EXPERIMENTAL)"""

from collections import deque
from textwrap import wrap
from matplotlib.ticker import ScalarFormatter
import matplotlib.pyplot as plt

__FIGURE_KWARGS = {'figsize': [6.4, 4.8], 'dpi': 200, \
    'tight_layout': {'pad': 1}}

__LINE_STYLES = ['solid', 'dashed', 'dashdot', 'dotted']

def __points_to_xy(points: list) -> tuple:
    # make sure all Xs are ints, because they will be sorted
    xslist = [int(p[0]) for p in points]
    yslist = [p[1] for p in points]
    return xslist, yslist

def __label(column: str, with_better=False) -> str:
    """Translate the name of a column to a label with a unit"""
    label_by_column = {
        'threads': '# of threads',
        'iodepth': 'iodepth',
        'bs': 'block size [B]',
        'lat_avg': 'latency [usec]',
        'lat_pctl_99.9': 'latency [usec]',
        'lat_pctl_99.99': 'latency [usec]',
        'bw_avg': 'bandwidth [Gb/s]',
        'cpuload': 'CPU load [%]'
    }
    lower = 'lower is better'
    higher = 'higher is better'
    better_by_column = {
        'lat_avg': lower,
        'lat_pctl_99.9': lower,
        'lat_pctl_99.99': lower,
        'bw_avg': higher
    }
    # If the column is not in the dictionary
    # the default return value is the raw name of the column.
    output = label_by_column.get(column, column)
    if with_better:
        output += '\n(' + better_by_column.get(column, column) + ')'
    return output

def draw_png(argx: str, argy: str, results: list, xscale: str,
             output_path: str, yaxis_max: int = None,
             suptitle: str = None, title: str = None) -> None:
    """draw a figure

    Generate a PNG file compiling all the provided results.

    Args:
        argx: an x-axis argument (`lib.figure.base.Figure.argx`)

        argy: an y-axis argument (`lib.figure.base.Figure.argy`)

        results: a list of results (`lib.figure.base.Figure.results`)

        xscale: an x-axis scale (`lib.figure.base.Figure.xscale`)

        output_path: a path where the output file will be written

        yaxis_max: a maximum value of the y-axis
          (`lib.figure.base.Figure.yaxis_max`)

        suptitle: a str with the suptitle for the output file

        title: a str with the title for the output file
    """
    # set output file size, padding and title
    fig = plt.figure(**__FIGURE_KWARGS)
    if suptitle is not None:
        suptitle = "\n".join(wrap(suptitle, 60))
        fig.suptitle(suptitle, fontsize='medium', y=0.90)
    # get a subplot
    plot = plt.subplot(1, 1, 1)
    if title is not None:
        plot.title.set_text(title)
        plot.title.set_fontsize(10)
    xticks = []
    line_styles = deque(__LINE_STYLES.copy())
    group_to_line_styles = {}
    for oneseries in results:
        # Pick a line style according to the group to which
        # the line belongs. If no group provided a default one is assumed.
        group = oneseries.get('group', 'default')
        if group in group_to_line_styles:
            line_style = group_to_line_styles[group]
        else:
            if len(line_styles) > 0:
                line_style = line_styles.popleft()
            else:
                raise Exception('Too many benchmarks.')
            group_to_line_styles[group] = line_style
        # draw series ony-by-one
        xslist, yslist = __points_to_xy(oneseries['points'])
        plot.plot(xslist, yslist, marker='.', linestyle=line_style,
                  label=oneseries['label'])
        # collect all existing x values
        xticks.extend(xslist)
    # make values unique (set) and sort them
    xticks = sorted(list(set(xticks)))
    # set the x-axis scale
    if xscale == "linear":
        plot.set_xscale(xscale)
    else:
        plot.set_xscale(xscale, base=2)
        plot.xaxis.set_major_formatter(ScalarFormatter())

    plot.set_xticks(xticks)
    plt.setp(plot.get_xticklabels(), rotation=45, ha='right')
    plot.set_xlabel(__label(argx))
    plot.set_ylabel(__label(argy, with_better=True))
    if yaxis_max is not None:
        plot.set_ylim(top=yaxis_max)
    plot.set_ylim(bottom=0)
    plot.legend(fontsize=10)
    plot.grid(True)

    plt.savefig(output_path)
    plt.close(fig)
