#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2020-2021, Intel Corporation
#

#
# csv2standardized.py -- standardize a CSV (EXPERIMENTAL)
#
# Convert a variety of CSV files into standardized-CSV or JSON format.
# The conversion includes:
# - renaming headers
# - converting units
#
# The standardized format columns are (in order):
# - threads - number of threads [1] (optional)
# - iodepth - number of operations submitted at once [1] (optional)
# - bs - block size [B]
# - ops - number of operations executed [1]
# - lat_* group (optional)
#   - lat_min - min latency [usec]
#   - lat_max - max latency [usec]
#   - lat_avg - avg. latency [usec]
#   - lat_stdev - stdev latency [usec]
#   - lat_pctl_99.0 - 99.0th percentile latency [usec]
#   - lat_pctl_99.9 - 99.9th percentile latency [usec]
#   - lat_pctl_99.99 - 99.99th percentile latency [usec] (optional)
#   - lat_pctl_99.999 - 99.999th percentile latency [usec] (optional)
# - bw_* group (optional)
#   - bw_min - min bandwidth [Gb/sec] (optional)
#   - bw_max - max bandwidth [Gb/sec] (optional)
#   - bw_avg - avg. bandwidth [Gb/sec]
# - cpuload (optional)
#

import argparse
import os.path
import pandas as pd

fio_input_names = [
    'bs', 'iodepth', 'threads', 'lat_min', 'lat_max', 'lat_avg', 'lat_stdev',
    'ops', 'lat_pctl_99.0', 'lat_pctl_99.9', 'lat_pctl_99.99',
    'lat_pctl_99.999', 'bw_avg', 'bw_min', 'bw_max', 'cpuload']

fio_nsec_2_usec_names = [
    'lat_min', 'lat_max', 'lat_avg', 'lat_stdev', 'lat_pctl_99.0',
    'lat_pctl_99.9', 'lat_pctl_99.99', 'lat_pctl_99.999']

fio_KiBps_2_Gbps_names = ['bw_avg', 'bw_min', 'bw_max']

KiBpbs_2_Gbps = 1024 * 8 / 1000 / 1000 / 1000

fio_output_names = [
    'threads', 'iodepth', 'bs', 'ops', 'lat_min', 'lat_max', 'lat_avg',
    'lat_stdev', 'lat_pctl_99.0', 'lat_pctl_99.9', 'lat_pctl_99.99',
    'lat_pctl_99.999', 'bw_min', 'bw_max', 'bw_avg', 'cpuload']

ib_lat_input_names = [
    'bs', 'ops', 'lat_min', 'lat_max', 'lat_mode', 'lat_avg', 'lat_stdev',
    'lat_pctl_99.0', 'lat_pctl_99.9']

ib_lat_output_names = [
    'bs', 'ops', 'lat_min', 'lat_max', 'lat_avg', 'lat_stdev', 'lat_pctl_99.0',
    'lat_pctl_99.9']

ib_bw_input_names = [
    'threads', 'iodepth', 'bs', 'ops', 'bw_peak', 'bw_avg', 'msg_rate']

ib_bw_output_names = [
    'threads', 'iodepth', 'bs', 'ops', 'bw_avg']

def main():
    parser = argparse.ArgumentParser(
        description='Standardize a CSV (EXPERIMENTAL)')
    parser.add_argument('csv_file', metavar='CSV_FILE',
        help='a CSV log file to process')
    parser.add_argument('--csv_type', metavar='CSV_TYPE', required=True,
        choices=['ib_lat', 'ib_bw', 'fio'], help='a type of the CSV file')
    parser.add_argument('--output_file', metavar='OUTPUT_FILE',
        default='output.csv', help='an output file')        
    args = parser.parse_args()

    if args.csv_type == 'ib_lat':
        df = pd.read_csv(args.csv_file, header=0, names=ib_lat_input_names)
        df = df.reindex(columns=ib_lat_output_names)
    elif args.csv_type == 'ib_bw':
        df = pd.read_csv(args.csv_file, header=0, names=ib_bw_input_names)
        df = df.reindex(columns=ib_bw_output_names)
        df = df.apply(lambda x: round(x, 2) \
            if x.name == 'bw_avg' else x)
    else: # fio
        df = pd.read_csv(args.csv_file, header=0, names=fio_input_names)
        # convert nsec to usec
        df = df.apply(lambda x: round(x / 1000, 2) \
            if x.name in fio_nsec_2_usec_names else x)
        # convert KiB/s to Gb/s
        df = df.apply(lambda x: round(x * KiBpbs_2_Gbps, 2) \
            if x.name in fio_KiBps_2_Gbps_names else x)
        df = df.reindex(columns=fio_output_names)
    
    _, ext = os.path.splitext(args.output_file)
    if ext == '.csv':
        df.to_csv(args.output_file, index=False)
    elif ext == '.json':
        df.to_json(args.output_file, orient='records')
    else:
        raise Exception(f"Unsupported output file extension: {ext}")

if __name__ == "__main__":
    main()

