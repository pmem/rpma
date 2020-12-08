#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2020, Intel Corporation
#

#
# csv_compare.py -- compare CSV files (EXPERIMENTAL)
#
# In order to compare both CSV are plotted on the same chart.
# XXX annotate data points / include data table for more fine-grained
# comparison.
# XXX include hostname for easier reporting.
#

import argparse
import pandas as pd
import matplotlib.pyplot as plt

column_to_label = {
    'bs':   'block size [B]',
    'lat':  'latency [nsec]',
    'bw':   'bandwidth [Gb/s]',
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
    }
}

def get_label(column):
    for key, label in column_to_label.items():
        if key in column:
            return label

def draw_column(ax, dfs, legend, x, y):
    xticks = None
    column_legend = []
    for df, df_name in zip(dfs, legend):
        if y not in df.columns:
            continue
        column_legend.append(df_name)
        # get xticks from the first data frame
        if xticks is None:
            xticks = df[x].tolist()
        df = df.set_index(x)
        # plot line on the subplot
        df[y].plot.line(ax=ax, rot=45)

    ax.set_xticks(xticks)
    ax.set_xlabel(get_label(x))
    ax.set_ylabel(get_label(y))
    ax.legend(column_legend)
    ax.grid(True)

def main():
    parser = argparse.ArgumentParser(
        description='Compare CSV files (EXPERIMENTAL)')
    parser.add_argument('csv_files', metavar='CSV_FILE', nargs='+',
        help='a CSV log file to process')
    parser.add_argument('--output_file', metavar='OUTPUT_FILE',
        default='compare.png', help='an output file')
    parser.add_argument('--output_layout', metavar='OUTPUT_LAYOUT',
        choices=layouts.keys(), required=True, help='an output file layout')
    parser.add_argument('--output_title', metavar='OUTPUT_TITLE',
        default='title', help='an output title')
    parser.add_argument('--legend', metavar='SERIES', nargs='+',
        help='a legend for the data series read from the CSV files')
    args = parser.parse_args()

    # read all CSV files
    dfs = []
    for csv_file in args.csv_files:
        df = pd.read_csv(csv_file)
        dfs.append(df)

    # get layout parameters
    layout = layouts.get(args.output_layout)
    nrows = layout.get('nrows')
    ncols =  layout.get('ncols')
    x = layout.get('x')

    # set output file size, padding and title
    fig = plt.figure(figsize=[6.4 * ncols, 4.8 * nrows], dpi=200, \
        tight_layout={'pad': 6})
    fig.suptitle(args.output_title)

    # draw all subplots
    for index, column in enumerate(layout.get('columns'), start=1):
        # get a subplot
        ax = plt.subplot(nrows, ncols, index)
        # set the subplot title
        ax.title.set_text(column)
        # draw CSVs column as subplot
        draw_column(ax, dfs, args.legend, x, column)

    # save the output file
    plt.savefig(args.output_file)

if __name__ == "__main__":
    main()

