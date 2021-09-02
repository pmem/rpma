#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2020-2021, Intel Corporation
#

#
# csv_compare.py -- compare CSV files (EXPERIMENTAL)
#
# In order to compare all CSV are plotted on the same chart.
# XXX include hostname for easier reporting.
#

import argparse
import os
import pandas as pd
import matplotlib.pyplot as plt

from PIL import Image
from textwrap import wrap
from matplotlib.ticker import ScalarFormatter

column_to_label = {
    'threads':  '# of threads',
    'iodepth':  'iodepth',
    'bs':       'block size [B]',
    'lat':      'latency [usec]',
    'bw':       'bandwidth [Gb/s]',
    'cpuload':  'CPU load [%]'
}

column_to_description = {
    'threads':  'threads={}',
    'iodepth':  'iodepth={}',
    'bs':       'block size={}B',
}

column_default = {
    'threads':  1,
    'iodepth':  1,
    'bs':       None,
}

dimensions = {'threads', 'iodepth', 'bs', 'cpuload'}

layouts = {
    'lat_avg': {
        'nrows': 1,
        'ncols': 1,
        'x': '<arg_axis>',
        'columns': [
            'lat_avg'
        ],
        'xscale': '<arg_xscale>',
    },
    'lat_pctls': {
        'nrows': 1,
        'ncols': 2,
        'x': '<arg_axis>',
        'columns': [
            # XXX When 99.999 percentile will be stabilized 99.99 can be
            # replaced with 99.999.
            'lat_pctl_99.9', 'lat_pctl_99.99'
        ],
        'xscale': '<arg_xscale>',
    },
    'lat_pctls_999': {
        'nrows': 1,
        'ncols': 2,
        'x': 'bs',
        'columns': [
            'lat_pctl_99.0', 'lat_pctl_99.9'
        ],
        'xscale': 'log',
    },
    'lat_pctls_99999': {
        'nrows': 1,
        'ncols': 2,
        'x': 'bs',
        'columns': [
            'lat_pctl_99.99', 'lat_pctl_99.999'
        ],
        'xscale': 'log',
    },
    'lat_pctl_99999': {
        'nrows': 1,
        'ncols': 1,
        'x': 'bs',
        'columns': [
            'lat_pctl_99.999'
        ],
        'xscale': 'log',
    },
    'lat_all': {
        'nrows': 4,
        'ncols': 2,
        'x': '<arg_axis>',
        'columns': [
            'lat_avg', 'lat_stdev',
            'lat_min', 'lat_max', 
            'lat_pctl_99.0', 'lat_pctl_99.9',
            'lat_pctl_99.99', 'lat_pctl_99.999'
        ],
        'xscale': '<arg_xscale>',
    },
    'bw': {
        'nrows': 1,
        'ncols': 1,
        'x': '<arg_axis>',
        'columns': [
            'bw_avg'
        ],
        'xscale': '<arg_xscale>',
    },
}

empty = {'lat_avg': [0], 'lat_pctl_99.9': [0], 'lat_pctl_99.999': [0], \
    'bs': [1], 'bw_avg': [0], 'threads': [1]}

def get_label(column):
    """Find a text label for an axis describing a provided CSV column.

    :param column: name of the CSV column
    :type column: str
    :return: a label for an axis
    :rtype: str
    """

    for key, label in column_to_label.items():
        if key in column:
            return label

def dfs_filter(dfs, df_names, column_list):
    """Filter out all pandas.DataFrame without required columns

    :param dfs: list of pandas.DataFrame objects to draw on the subplot
    :type dfs: list[pandas.DataFrame]
    :param df_names: a list of human readable descriptions for dfs
    :type df_names: list[str]
    :param column_list: a list of required columns
    :type column_list: list[str]
    :return: a list of pandas.DataFrame and their names
    :rtype: list[pandas.DataFrame], list[str]
    """
    dfs_out = []
    df_names_out = []
    # loop over all (pandas.DataFrame, str) pairs
    for df, df_name in zip(dfs, df_names):
        has_all = True
        for column in column_list:
            if column not in df.columns:
                has_all = False
                break
        # if DataFrame does not have all specified columns just skip
        # the DataFrame
        if not has_all:
            continue
        # append the DataFrame and its name to the outputs
        dfs_out.append(df)
        df_names_out.append(df_name)

    return dfs_out, df_names_out

def dfs_all_values(dfs, column):
    """Find all possible values of a column in the pandas.DataFram list

    :param dfs: list of pandas.DataFrame objects to draw on the subplot
    :type dfs: list[pandas.DataFrame]
    :param column: a columns
    :type column: str
    :return: a sorted list of possible values from the column
    :rtype: list[values]
    """
    values = []
    # loop over all (pandas.DataFrame, str) pairs
    for df in dfs:
        values.extend(df[column].tolist())

    # set() removes duplicates
    # sorted() converts Set to List and sort the elements
    return sorted(set(values))

def draw_plot(ax, dfs, legend, x, y, xscale, yaxis_max):
    """Draw multiple lines y(x) using data from the dfs list on the ax subplot.

    :param ax: an axes (subplot)
    :type ax: matplotlib.axes
    :param dfs: list of pandas.DataFrame objects to draw on the subplot
    :type dfs: list[pandas.DataFrame]
    :param legend: a list of human readable descriptions for dfs
    :type legend: list[str]
    :param x: a column to be drawn on the x-axis
    :type x: str
    :param y: a column to be drawn on the y-axis
    :type y: str
    :param xscale: a x-axis scale
    :type xscale: str
    """
    xticks = dfs_all_values(dfs, x)
    # loop over all pandas.DataFrame objects
    for df in dfs:
        # setting the x-column as an index is required to draw the y-column
        # as a function of x argument
        df = df.set_index(x)
        # plot line on the subplot
        df[y].plot.line(ax=ax, rot=45, marker='.')

    if xscale == "linear":
        ax.set_xscale(xscale)
    else:
        ax.set_xscale(xscale, base=2)
        ax.xaxis.set_major_formatter(ScalarFormatter())

    ax.set_xticks(xticks)
    ax.set_xlabel(get_label(x))
    ax.set_ylabel(get_label(y))
    ax.set_ylim(bottom=0)
    if yaxis_max is not None:
        ax.set_ylim(top=float(yaxis_max))
    ax.legend(legend, fontsize=6)
    ax.grid(True)

def draw_table(ax, dfs, legend, x, y):
    """Draw a table of all data used to chart y(x)

    :param ax: an axes (subplot)
    :type ax: matplotlib.axes
    :param dfs: list of pandas.DataFrame objects to draw on the subplot
    :type dfs: list[pandas.DataFrame]
    :param legend: a list of human readable descriptions for dfs
    :type legend: list[str]
    :param x: a column to be drawn on the x-axis
    :type x: str
    :param y: a column to be drawn on the y-axis
    :type y: str
    """
    col_labels = dfs_all_values(dfs, x)
    column_legend = []
    cell_text = []
    # loop over all pandas.DataFrame objects
    for df in dfs:
        # to allow query y(x) easily
        df = df.set_index(x)
        df_row = df[y]
        # build a row with filled blanks '-'
        row = ["{:.2f}".format(df_row[column]) if column in df_row.index else '-' \
            for column in col_labels]
        cell_text.append(row)

    ax.axis('tight')
    ax.axis('off')
    ax.table(cellText=cell_text, rowLabels=legend, colLabels=col_labels, \
        loc='top')

def get_content_height(im, ncols):
    width, height = im.size
    # pick a vertical line where the content is expected
    x = width / (ncols + 1)
    for y in range(height - 1, 0, -1):
        pixel = im.getpixel((x, y))
        if pixel[0] != 255 or pixel[1] != 255 or pixel[2] != 255:
            return y + 1
    return 0

def crop_to_content(file, ncols):
    # open the file
    im = Image.open(file)
    # calculate the crop parameters
    width, height = im.size
    left = 0
    top = 0
    right = width
    bottom = get_content_height(im, ncols)
    bottom += 100
    # crop and save the output file
    im = im.crop((left, top, right, bottom))
    im.save(file)

def split_in_half(input, output, top):
    # open the file
    im = Image.open(input)
    # calculate the crop parameters
    width, height = im.size
    left = 0
    top = int(top * height)
    right = width
    bottom = int(top + 0.5 * height)
    # crop and save the output file
    im = im.crop((left, top, right, bottom))
    im.save(output)

def get_const_param(dfs, column):
    df = dfs[0]
    value = column_default[column]
    if column in df.columns:
        value = df.at[0, column]
    return value

def main():
    parser = argparse.ArgumentParser(
        description='Compare CSV files (EXPERIMENTAL)')
    parser.add_argument('csv_files', metavar='CSV_FILE', nargs='+',
        help='a CSV log file to process')
    parser.add_argument('--output_file', metavar='OUTPUT_FILE',
        default='compare.png', help='an output file')
    parser.add_argument('--output_layout', metavar='OUTPUT_LAYOUT',
        choices=layouts.keys(), required=True, help='an output file layout')
    parser.add_argument('--arg_axis', metavar='ARG_AXIS',
        choices=dimensions, required=False,
        help='an axis for layouts which requires to pick one')
    parser.add_argument('--yaxis_max', metavar='YMAX',
        default=None, help='a y-axis max value')
    parser.add_argument('--arg_xscale', metavar='XSCALE',
        choices=['linear', 'log'], required=False, help='an x-axis scale')
    parser.add_argument('--output_with_tables', action='store_true',
        help='an output file layout')
    parser.add_argument('--output_title', metavar='OUTPUT_TITLE',
        default='title', help='an output title')
    parser.add_argument('--legend', metavar='SERIES', nargs='+',
        help='a legend for the data series read from the CSV files')
    parser.add_argument('--legend_from_file_name_comment', action='store_true',
        help='generate a legend from the file name comments __COMMENT__')
    args = parser.parse_args()

    # generate or validate the legend
    if args.legend_from_file_name_comment:
        args.legend = []
        for fname in args.csv_files:
            comment = fname.split('__')
            if len(comment) != 3 or len(comment[1]) == 0:
                args.legend.append(fname)
            args.legend.append(comment[1])
    elif args.legend is None:
        args.legend = args.csv_files
    elif len(args.legend) != len(args.csv_files):
        raise Exception(
            'The number of legend entries does not match the number of CSV files')

    # read all CSV files
    dfs = []
    for csv_file in args.csv_files:
        try:
            df = pd.read_csv(csv_file)
            dfs.append(df)
        except:
            dfs.append(pd.DataFrame.from_dict(empty))


    # get a layout
    layout = layouts[args.output_layout]

    # fill out an optional axis with the provided argument
    if args.output_layout in ['bw', 'lat_avg', 'lat_pctls', 'lat_all']:
        if args.arg_axis is None:
            raise Exception('The layout requires --arg_axis')
        elif args.arg_xscale is None:
            raise Exception('The layout requires --arg_xscale')
        layout['x'] = args.arg_axis
        layout['xscale'] = args.arg_xscale

    # get layout parameters
    columns = layout['columns']
    nrows = layout['nrows']
    ncols =  layout['ncols']
    x = layout['x']
    xscale = layout['xscale']

    # prepare indices
    indices = list(range(1, len(columns) + 1))
    # make space for tables
    if args.output_with_tables:
        indices = [i + int((i - 1) / ncols) * ncols for i in indices]
        nrows *= 2

    # set output file size, padding and title
    fig = plt.figure(figsize=[6.4 * ncols, 4.8 * nrows], dpi=200, \
        tight_layout={'pad': 6})
    fig.suptitle("\n".join(wrap(args.output_title, \
        (60 if ncols < 2 else 50 * ncols))), \
        fontsize=('xx-large' if ncols > 1 else 'medium'), \
        y=(0.90 if nrows < 4 else 0.98))

    # draw all subplots
    for index, column in zip(indices, columns):
        # get a subplot
        ax = plt.subplot(nrows, ncols, index)
        
        # filter out all DataFrames without required columns
        dfs_filtered, dfs_names_filtered = \
            dfs_filter(dfs, args.legend, [x, column])
        if len(dfs_filtered) == 0:
            ax.title.set_text(column)
            continue

        # prepare constant parameters description
        const_params = []
        for param in column_default.keys():
            if param == x:
                continue
            value = get_const_param(dfs_filtered, param)
            if value is not None:
                const_params.append(column_to_description[param].format(value))

        # set the subplot title
        if len(const_params) == 0:
            title = column
        else:
            title = "{} [{}]".format(column, ', '.join(const_params))
        ax.title.set_text(title)

        # draw CSVs column as subplot
        draw_plot(ax, dfs_filtered, dfs_names_filtered, x, column, xscale, \
            args.yaxis_max)
        if args.output_with_tables:
            # get a subplot just beneath the subplot with the chart
            ax = plt.subplot(nrows, ncols, index + ncols)
            # put a table
            draw_table(ax, dfs_filtered, dfs_names_filtered, x, column)

    # save the output file
    plt.savefig(args.output_file)

    # crop
    if args.output_layout in \
            ['bw', 'lat_avg', 'lat_pctls', 'lat_pctls_999', 'lat_pctls_99999', 'lat_pctl_99999']:
        crop_to_content(args.output_file, layout['ncols'])

    # split long files
    if args.output_layout == 'lat_all':
        name, ext = os.path.splitext(args.output_file)
        output_file_1 = "{}_1{}".format(name, ext)
        output_file_2 = "{}_2{}".format(name, ext)
        split_in_half(args.output_file, output_file_2, 0.5)
        crop_to_content(output_file_2, layout['ncols'])
        split_in_half(args.output_file, output_file_1, 0.0)
        crop_to_content(output_file_1, layout['ncols'])
        os.remove(args.output_file)

if __name__ == "__main__":
    main()
