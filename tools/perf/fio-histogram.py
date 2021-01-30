#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2020, Intel Corporation
#

#
# fio-histogram.py -- generate a histogram from a FIO log (EXPERIMENTAL)
#
# To generate a FIO log you have to use write_bw_log, write_lat_log
# or write_iops_log option. Please see the FIO manual for details.
#

import argparse
import pandas as pd
import matplotlib.pyplot as plt

type_to_xlabel = {
    'lat': 'latency [nsec]',
    'bw': 'bandwidth [KiB/s]',
    'iops': 'IOPS'
}

def main():
    parser = argparse.ArgumentParser(
        description='Generate a histogram from a FIO log (EXPERIMENTAL)')
    parser.add_argument('log_file', metavar='LOG_FILE',
        help='a FIO log file to process')
    parser.add_argument('--log_type', metavar='LOG_TYPE',
        choices=['lat', 'bw', 'iops'], default='lat',
        help='a FIO log type')
    parser.add_argument('--output_file', metavar='OUTPUT_FILE',
        default='histogram.png', help='an output file')
    parser.add_argument('--chart_title', metavar='CHART_TITLE',
        default='histogram', help='a chart title')
    parser.add_argument('--chart_yscale', metavar='CHART_YSCALE',
        choices=['linear', 'log'], default='log', help='a chart yscale')
    parser.add_argument('--outliers_cut_off', metavar='OUTLIERS_CUT_OFF',
        type=int, help='an outliers cut-off value')
    args = parser.parse_args()

    df = pd.read_csv(args.log_file, header=0,
        names=['msec', 'value', 'ddir', 'bs', 'offset', 'prio'])
    if args.outliers_cut_off is not None:
        # filter out outliers
        df = df[df['value'] < args.outliers_cut_off]
    fig, ax = plt.subplots()
    df.hist(ax=ax, column='value')
    ax.set_yscale(args.chart_yscale)
    ax.set_ylabel('frequency')
    ax.set_xlabel(type_to_xlabel.get(args.log_type))
    plt.title(args.chart_title)
    plt.savefig(args.output_file)

if __name__ == "__main__":
    main()

