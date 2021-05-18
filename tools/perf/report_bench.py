#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021, Intel Corporation
#

#
# report_bench.py -- run report-specific benchmarks (EXPERIMENTAL)
#

import argparse
import json
import os

from lib.common import *
from lib.Bench import *

Parser = argparse.ArgumentParser(
    description='Run report-specific benchmarks (EXPERIMENTAL)')
subparsers = Parser.add_subparsers(dest='command', required=True)
parser_f = subparsers.add_parser('run', help='start a fresh benchmark')
parser_f.add_argument('--config', type=json_from_file, required=True,
    help='a platform configuration file')
# XXX it should be possible to provide multiple files or an experiment.json
# file providing a set of benchmarks to perform
parser_f.add_argument('--figure', type=json_from_file, required=True,
    help='a figure file describing the benchmark')
parser_f.add_argument('--result_dir', type=dir_path, required=True,
    help='an output directory')
parser_c = subparsers.add_parser('continue', help='continue the benchmark')
parser_c.add_argument('--bench', type=json_from_file, required=True,
    help='a bench.json file of an interrupted benchmark')

def cmd_run(args):
    bench = Bench.new(args.config, args.figure, args.result_dir)
    bench.run()

def cmd_continue(args):
    bench = Bench.carry_on(args.bench)
    bench.run()

def main():
    args = Parser.parse_args()
    # call a command-specific function by its name
    globals()['cmd_' + args.command](args)
    print('Done.')

if __name__ == '__main__':
    main()
