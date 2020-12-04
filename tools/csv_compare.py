#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2020, Intel Corporation
#

#
# csv_compare.py -- compare CSV files (EXPERIMENTAL)
#
# In order to compare both CSV are plotted on the same chart.
# XXX xticks overlap ugly. They should be rotated. Please see the comment
# in the code.
# XXX annotate data points / include data table for more fine-grained
# comparison.
# XXX legend should be more humanredable.
# XXX plot title can be prettier.
# XXX include hostname for easier reporting.
# XXX it will be easier to compare both data sets if all columns will be drawn
# into a single file.
#

import argparse
import pandas as pd
import matplotlib.pyplot as plt

names = [
    'lat_min', 'lat_max', 'lat_avg', 'lat_stddev', 'lat_pctl_99.0',
    'lat_pctl_99.9', 'lat_pctl_99.99', 'lat_pctl_99.999']

def main():
    parser = argparse.ArgumentParser(
        description='Compare CSV files (EXPERIMENTAL)')
    parser.add_argument('csv_files', metavar='CSV_FILE', nargs='+',
        help='a CSV log file to process')
    parser.add_argument('--output_file', metavar='OUTPUT_FILE',
        default='compare.png', help='an output file')
    parser.add_argument('--column', metavar='COLUMN', choices=names,
        help='a CSV column to compare')
    args = parser.parse_args()

    fig, ax = plt.subplots()
    xticks = None
    for csv_file in args.csv_files:
        df = pd.read_csv(csv_file)
        if xticks is None:
            xticks = df['bs'].tolist()
        df = df.set_index('bs')
        # XXX with rot=45 looks a lot better but xlabel is obscured
        df[args.column].plot.line(ax=ax)

    ax.set_xticks(xticks)
    ax.set_xlabel('block size [B]')
    ax.set_ylabel('latency [usec]')
    plt.title(args.column)
    ax.legend(args.csv_files)
    plt.savefig(args.output_file)

if __name__ == "__main__":
    main()

