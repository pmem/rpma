#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2020-2021, Intel Corporation
#

#
# fio-json2csv.py -- generate a CSV from FIO JSON (EXPERIMENTAL)
#

import argparse
import csv
import json
import yaml

def main():
    parser = argparse.ArgumentParser(
        description='Generate a CSV from FIO JSON (EXPERIMENTAL)')
    parser.add_argument('json_file', metavar='JSON_FILE',
        help='a FIO json file to process')
    parser.add_argument('--op', metavar='OP',
        choices=['read', 'write'], default='read',
        help='a FIO operation to extract')
    parser.add_argument('--extra', type=yaml.safe_load,
        help='additional key:value pairs to append to the output in form of yaml dictionary e.g. "{key: value}"')
    parser.add_argument('--output_file', metavar='CSV_FILE',
        default='output.csv', help='an output file')
    args = parser.parse_args()

    with open(args.json_file) as json_file, \
            open(args.output_file, 'w') as csv_file:
        # read JSON file
        data = json.load(json_file)
        job = data['jobs'][0]
        options = job['job options']
        op = job[args.op]
        lat_ns = op['lat_ns']
        # prepare data for writing
        csv_columns = ['bs']
        csv_data = {'bs': options['bs']}
        # append numjobs
        for column in ['iodepth', 'numjobs']:
            csv_columns.append(column)
            csv_data[column] = options[column] if column in options else "1"
        for k,v in lat_ns.items():
            if k == "percentile" or k == "bins":
                continue
            column = 'lat_ns_' + k
            csv_columns.append(column)
            csv_data[column] = v
        # prepare percentiles for writing
        for k, v in lat_ns['percentile'].items():
            column = 'lat_ns_pctl_' + k
            csv_columns.append(column)
            csv_data[column] = v
        # prepare bw for writing
        bw_columns = ['bw', 'bw_min', 'bw_max']
        for column in bw_columns:
            csv_columns.append(column)
            csv_data[column] = op[column]
        # append extra key:value pairs
        if args.extra is not None:
            for k, v in args.extra.items():
                csv_columns.append(k)
                csv_data[k] = v
        # write CSV file
        writer = csv.DictWriter(csv_file, fieldnames=csv_columns)
        writer.writeheader()
        writer.writerow(csv_data)

if __name__ == "__main__":
    main()
