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
# - iops_* group (optional)
#   - iops_min - min [operations/sec] (optional)
#   - iops_max - max [operations/sec] (optional)
#   - iops_avg - avg. [operations/sec]
# - cpuload (optional)
#

import argparse
import json
import os.path

import lib.format as fmt

FORMATS = {
    'ib_lat': fmt.IbReadLatFormat,
    'ib_bw': fmt.IbReadBwFormat,
}

def main():
    parser = argparse.ArgumentParser(
        description='Standardize a CSV (EXPERIMENTAL)')
    parser.add_argument('csv_files', metavar='CSV_FILE', nargs='+',
        help='a CSV log file(s) to process')
    parser.add_argument('--csv_type', metavar='CSV_TYPE', required=True,
        choices=['ib_lat', 'ib_bw', 'fio'], help='a type of the CSV file')
    parser.add_argument('--keys', metavar='KEY', nargs='+',
        help='keys enumerating the input csv_files in the output_file')
    parser.add_argument('--output_file', metavar='OUTPUT_FILE',
        default='output.csv', help='an output file')
    args = parser.parse_args()

    def read_csv(csv_file):
        format = FORMATS.get(args.csv_type, fmt.FioFormat)
        return format.read_csv(csv_file)

    dfs = [read_csv(csv_file) for csv_file in args.csv_files]
    _, ext = os.path.splitext(args.output_file)
    if ext == '.csv':
        if len(dfs) != 1:
            raise ArithmeticError(
                "csv output_file does not allow to store more than one input file")
        dfs[0].to_csv(args.output_file, index=False)
    elif ext == '.json':
        if args.keys is None or len(args.keys) == 0:
            # without keys only a single input file can be stored
            if len(dfs) != 1:
                raise ArithmeticError(
                    "to store multiple csv_files you have to provide keys")
            dfs[0].to_json(args.output_file, orient='records')
        else:
            if len(args.keys) != len(dfs):
                raise ArithmeticError(
                    "the number of keys have to be equal to the number of csv_files")
            # prepare the output dict()
            output = {key: df.to_dict(orient='records')
                for key, df in zip(args.keys, dfs)}
            # write the constructed dict() to the output_file
            with open(args.output_file, 'w', encoding='utf-8') as file:
                json.dump(output, file)
    else:
        raise Exception(f"Unsupported output file extension: {ext}")

if __name__ == "__main__":
    main()

