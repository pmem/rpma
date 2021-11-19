#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2020-2021, Intel Corporation
#

"""fio-json2csv.py -- generate a CSV from FIO JSON (EXPERIMENTAL)"""

import argparse
import csv
import yaml

from lib.format import FioFormat

def main():
    parser = argparse.ArgumentParser(
        description='Generate a CSV from FIO JSON (EXPERIMENTAL)')
    parser.add_argument('json_file', metavar='JSON_FILE',
                        help='a FIO json file to process')
    parser.add_argument('--op', metavar='OP', choices=['read', 'write'],
                        default='read', help='a FIO operation to extract')
    parser.add_argument('--extra', type=yaml.safe_load,
                        help='additional key:value pairs to append ' \
                        'to the output in form of yaml dictionary ' \
                        'e.g. "{key: value}"')
    parser.add_argument('--output_file', metavar='CSV_FILE',
                        default='output.csv', help='an output file')
    args = parser.parse_args()

    # read JSON file
    with open(args.json_file, 'r', encoding='utf-8') as json_file:
        json_str = json_file.read()
    # prepare data for writing
    op_result = FioFormat.parse(json_str, FioFormat.Output.DUMP)[args.op]
    # append extra key:value pairs
    op_result = {**op_result, **args.extra}
    # write CSV file
    with open(args.output_file, 'w', encoding='utf-8') as csv_file:
        writer = csv.DictWriter(csv_file, fieldnames=op_result.keys())
        writer.writeheader()
        writer.writerow(op_result)

if __name__ == "__main__":
    main()
