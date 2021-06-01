#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021, Intel Corporation
#

#
# report_create.py -- generate a performance report (EXPERIMENTAL)
#

import argparse

from jinja2 import Environment, FileSystemLoader

from lib.common import *
from lib.Bench import *
from lib.Report import *

Parser = argparse.ArgumentParser(
    description='Generate a performance report (EXPERIMENTAL)')
Parser.add_argument('--bench', type=json_from_file, required=True,
    help='a bench.json file of a completed benchmark')
Parser.add_argument('--output', default='report',
    help='a name of the output file')
SEARCHPATH = 'templates'
Loader = FileSystemLoader(SEARCHPATH)
Env = Environment(loader=Loader)

def main():
    args = Parser.parse_args()
    bench = Bench.carry_on(args.bench)
    bench.check_completed()
    report = Report(Loader, Env, bench)
    report.create(args.output)
    print("Done.")

if __name__ == "__main__":
    main()
