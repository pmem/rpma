#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021, Intel Corporation
#

#
# report_create.py
#

"""generate a performance report (EXPERIMENTAL)

Restore the lib.Bench object, check whether it has completed the ordered benchmarks. Create a lib.Report object based on the lib.Bench results and provided report.json configuration file. Use the lib.Report object to generate a report document.

  For usage:

  $ ./report_create.py --help
"""

import argparse

from shutil import copy, SameFileError
from jinja2 import Environment, FileSystemLoader

from lib.common import json_from_file
from lib.bench import Bench
from lib.Report import Report

PARSER = argparse.ArgumentParser(
    description='Generate a performance report (EXPERIMENTAL)')
PARSER.add_argument('--bench', type=json_from_file, required=True,
                    help='a bench.json file of a completed benchmark')
PARSER.add_argument('--report', type=json_from_file, required=True,
                    help='''a report.json configuration file provides textual content and configures the visualization process''')
PARSER.add_argument('--output', default='report',
                    help='a name of the output file')
SEARCHPATH = 'templates'
LOADER = FileSystemLoader(SEARCHPATH)
ENV = Environment(loader=LOADER)

def main():
    """I'm main"""
    args = PARSER.parse_args()
    bench = Bench.carry_on(args.bench)
    bench.check_completed()
    report = Report(LOADER, ENV, bench, args.report)
    try:
        # copying report.json file to the result dir for future reference
        copy(args.report['input_file'], bench.result_dir)
    except SameFileError:
        # do nothing if src and dst specify the same file
        pass
    report.create(args.output)
    print("Done.")

if __name__ == "__main__":
    main()
