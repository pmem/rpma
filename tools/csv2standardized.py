#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2020, Intel Corporation
#

#
# csv2standardized.py -- standardize a CSV (EXPERIMENTAL)
#
# Convert a variety of CSV files into standardized-CSV format.
# The conversion includes:
# - renaming headers
# - converting units
#
# The standardized-CSV format columns are (in order):
# - bs - block size [B]
# - lat_min - min latency [usec]
# - lat_max - max latency [usec]
# - lat_mode - mode latency [usec] (optional)
# - lat_avg - avg latency [usec]
# - lat_stdev - stdev latency [usec]
# - lat_pctl_99.0 - 99.0th percentile latency [usec]
# - lat_pctl_99.9 - 99.9th percentile latency [usec]
# - lat_pctl_99.99 - 99.99th percentile latency [usec] (optional)
# - lat_pctl_99.999 - 99.999th percentile latency [usec] (optional)
#

import argparse
import pandas as pd

fio_input_names = [
    'bs', 'lat_min', 'lat_max', 'lat_avg', 'lat_stdev', 'ops',
    'lat_pctl_99.0', 'lat_pctl_99.9', 'lat_pctl_99.99', 'lat_pctl_99.999']

fio_nsec_2_usec_names = [
    'lat_min', 'lat_max', 'lat_avg', 'lat_stdev', 'lat_pctl_99.0',
    'lat_pctl_99.9', 'lat_pctl_99.99', 'lat_pctl_99.999']

fio_names = [
    'bs', 'ops', 'lat_min', 'lat_max', 'lat_avg', 'lat_stdev', 
    'lat_pctl_99.0', 'lat_pctl_99.9', 'lat_pctl_99.99', 'lat_pctl_99.999']

ib_input_names = [
    'bs', 'ops', 'lat_min', 'lat_max', 'lat_mode', 'lat_avg', 'lat_stdev',
    'lat_pctl_99.0', 'lat_pctl_99.9']

ib_names = [
    'bs', 'ops', 'lat_min', 'lat_max', 'lat_mode', 'lat_avg', 'lat_stdev',
    'lat_pctl_99.0', 'lat_pctl_99.9']

def main():
    parser = argparse.ArgumentParser(
        description='Standardize a CSV (EXPERIMENTAL)')
    parser.add_argument('csv_file', metavar='CSV_FILE',
        help='a CSV log file to process')
    parser.add_argument('--csv_type', metavar='CSV_TYPE', required=True,
        choices=['ib', 'fio'], help='a type of the CSV file')
    parser.add_argument('--output_file', metavar='OUTPUT_FILE',
        default='output.csv', help='an output file')
    args = parser.parse_args()

    if args.csv_type == 'ib':
        df = pd.read_csv(args.csv_file, header=0, names=ib_input_names)
        df = df.reindex(columns=ib_names)
    else: # fio
        df = pd.read_csv(args.csv_file, header=0, names=fio_input_names)
        # convert nsec to usec
        df = df.apply(lambda x: round(x / 1000, 2) \
            if x.name in fio_nsec_2_usec_names else x)
        df = df.reindex(columns=fio_names)
    df.to_csv(args.output_file, index=False)

if __name__ == "__main__":
    main()

