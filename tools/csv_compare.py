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

layouts = {
    'lat': {
        'nrows': 4,
        'ncols': 2,
        'columns': [
            'lat_avg', 'lat_stdev',
            'lat_min', 'lat_max', 
            'lat_pctl_99.0', 'lat_pctl_99.9',
            'lat_pctl_99.99', 'lat_pctl_99.999'
        ]
    }
}

def draw_column(ax, dfs, column, legend):
    xticks = None
    column_legend = []
    for df, df_name in zip(dfs, legend):
        if column not in df.columns:
            continue
        column_legend.append(df_name)
        # get xticks from the first data frame
        if xticks is None:
            xticks = df['bs'].tolist()
        df = df.set_index('bs')
        # plot line on the subplot
        df[column].plot.line(ax=ax, rot=45)

    ax.set_xticks(xticks)
    ax.set_xlabel('block size [B]')
    ax.set_ylabel('latency [usec]')
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

    # set output file size, padding and title
    fig = plt.figure(figsize=[12.8, 19.2], dpi=200, tight_layout={'pad': 6})
    fig.suptitle(args.output_title)

    # get layout parameters dict
    layout = layouts.get(args.output_layout)
    # draw all subplots
    for index, column in enumerate(layout.get('columns'), start=1):
        # get a subplot
        ax = plt.subplot(layout.get('nrows'), layout.get('ncols'), index)
        # set the subplot title
        ax.title.set_text(column)
        # draw CSVs column as subplot
        draw_column(ax, dfs, column, args.legend)

    # save the output file
    plt.savefig(args.output_file)

if __name__ == "__main__":
    main()

