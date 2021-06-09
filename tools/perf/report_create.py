#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021, Intel Corporation
#

"""report_create.py -- generate a performance report (EXPERIMENTAL)"""

import argparse

from jinja2 import Environment, FileSystemLoader

from lib.common import json_from_file
from lib.Bench import Bench
from lib.Report import Report

PARSER = argparse.ArgumentParser(
    description='Generate a performance report (EXPERIMENTAL)')
PARSER.add_argument('--bench', type=json_from_file, required=True,
                    help='a bench.json file of a completed benchmark')
PARSER.add_argument('--output', default='report',
                    help='a name of the output file')
SEARCHPATH = 'templates'
LOADER = FileSystemLoader(SEARCHPATH)
ENV = Environment(loader=LOADER)

def main():
    """
    Restore the Bench object, check whether it has completed the benchmarking,
    and create a report based on the results.
    """
    args = PARSER.parse_args()
    bench = Bench.carry_on(args.bench)
    bench.check_completed()
    report = Report(LOADER, ENV, bench)
    report.create(args.output)
    print("Done.")

if __name__ == "__main__":
    main()
