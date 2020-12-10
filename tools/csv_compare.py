#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2020, Intel Corporation
#

#
# csv_compare.py -- compare CSV files (EXPERIMENTAL)
#
# In order to compare all CSV are plotted on the same chart.
# XXX include hostname for easier reporting.
#

import argparse
import pandas as pd
import matplotlib.pyplot as plt

column_to_label = {
    'threads':  '# of threads',
    'bs':       'block size [B]',
    'lat':      'latency [nsec]',
    'bw':       'bandwidth [Gb/s]',
}

layouts = {
    'lat': {
        'nrows': 4,
        'ncols': 2,
        'x': 'bs',
        'columns': [
            'lat_avg', 'lat_stdev',
            'lat_min', 'lat_max', 
            'lat_pctl_99.0', 'lat_pctl_99.9',
            'lat_pctl_99.99', 'lat_pctl_99.999'
        ]
    },
    'bw_vs_bs': {
        'nrows': 1,
        'ncols': 1,
        'x': 'bs',
        'columns': [
            'bw_avg'
        ]
    },
    'bw_vs_th': {
        'nrows': 1,
        'ncols': 1,
        'x': 'threads',
        'columns': [
            'bw_avg'
        ]
    }
}

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

def draw_plot(ax, dfs, legend, x, y):
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
    """
    xticks = dfs[0][x].tolist()
    # loop over all pandas.DataFrame objects
    for df in dfs:
        # setting the x-column as an index is required to draw the y-column
        # as a function of x argument
        df = df.set_index(x)
        # plot line on the subplot
        df[y].plot.line(ax=ax, rot=45)

    ax.set_xticks(xticks)
    ax.set_xlabel(get_label(x))
    ax.set_ylabel(get_label(y))
    ax.legend(legend)
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
    col_labels = dfs[0][x].tolist()
    column_legend = []
    cell_text = []
    # loop over all pandas.DataFrame objects
    for df in dfs:
        # plot line on the subplot
        cell_text.append(df[y].tolist())

    ax.axis('tight')
    ax.axis('off')
    ax.table(cellText=cell_text, rowLabels=legend, colLabels=col_labels, \
        loc='top')

def main():
    parser = argparse.ArgumentParser(
        description='Compare CSV files (EXPERIMENTAL)')
    parser.add_argument('csv_files', metavar='CSV_FILE', nargs='+',
        help='a CSV log file to process')
    parser.add_argument('--output_file', metavar='OUTPUT_FILE',
        default='compare.png', help='an output file')
    parser.add_argument('--output_layout', metavar='OUTPUT_LAYOUT',
        choices=layouts.keys(), required=True, help='an output file layout')
    parser.add_argument('--output_with_tables', action='store_true',
        help='an output file layout')
    parser.add_argument('--output_title', metavar='OUTPUT_TITLE',
        default='title', help='an output title')
    parser.add_argument('--legend', metavar='SERIES', nargs='+',
        help='a legend for the data series read from the CSV files')
    args = parser.parse_args()

    # validate the legend
    if args.legend is None:
        args.legend = args.csv_files
    elif len(args.legend) != len(args.csv_files):
        raise Exception(
            'The number of legend entries does not match the number of CSV files')

    # read all CSV files
    dfs = []
    for csv_file in args.csv_files:
        df = pd.read_csv(csv_file)
        dfs.append(df)

    # get layout parameters
    layout = layouts.get(args.output_layout)
    columns = layout.get('columns')
    nrows = layout.get('nrows')
    ncols =  layout.get('ncols')
    x = layout.get('x')

    # prepare indices
    indices = list(range(1, len(columns) + 1))
    # make space for tables
    if args.output_with_tables:
        indices = [i + int((i - 1) / ncols) * ncols for i in indices]
        nrows *= 2

    # set output file size, padding and title
    fig = plt.figure(figsize=[6.4 * ncols, 4.8 * nrows], dpi=200, \
        tight_layout={'pad': 6})
    fig.suptitle(args.output_title)

    # draw all subplots
    for index, column in zip(indices, columns):
        # get a subplot
        ax = plt.subplot(nrows, ncols, index)
        # set the subplot title
        ax.title.set_text(column)
        # filter out all DataFrames without required columns
        dfs_filtered, dfs_names_filtered = \
            dfs_filter(dfs, args.legend, [x, column])
        if len(dfs_filtered) == 0:
            continue
        # draw CSVs column as subplot
        draw_plot(ax, dfs_filtered, dfs_names_filtered, x, column)
        if args.output_with_tables:
            # get a subplot just beneath the subplot with the chart
            ax = plt.subplot(nrows, ncols, index + ncols)
            # put a table
            draw_table(ax, dfs_filtered, dfs_names_filtered, x, column)

    # save the output file
    plt.savefig(args.output_file)

if __name__ == "__main__":
    main()

