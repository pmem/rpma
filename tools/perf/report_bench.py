#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021, Intel Corporation
#

"""report_bench.py -- run report-specific benchmarks (EXPERIMENTAL)"""

import argparse

from lib.common import json_from_file, dir_path
from lib.Bench import Bench

PARSER = argparse.ArgumentParser(
    description='Run report-specific benchmarks (EXPERIMENTAL)')
SUBPARSERS = PARSER.add_subparsers(dest='command')
# Python >= 3.7 accepts 'required' kwarg. For older versions, it is validated
# manually.
SUBPARSERS.required = True
PARSER_R = SUBPARSERS.add_parser('run', help='start a new benchmark')
PARSER_R.add_argument('--config', type=json_from_file, required=True,
                      help='a platform configuration file')
# XXX it should be possible to provide multiple files or an experiment.json
# file providing a set of benchmarks to be run
PARSER_R.add_argument('--figure', type=json_from_file, required=True,
                      help='a figure file describing the benchmark')
PARSER_R.add_argument('--result_dir', type=dir_path, required=True,
                      help='an output directory')
PARSER_C = SUBPARSERS.add_parser('continue', help='continue the benchmark')
PARSER_C.add_argument('--bench', type=json_from_file, required=True,
                      help='a bench.json file of an interrupted benchmark')

def main():
    """
    Create new or restore the Bench object and try to run all defined
    benchmarks.
    """
    args = PARSER.parse_args()
    if args.command == "run":
        bench = Bench.new(args.config, args.figure, args.result_dir)
    elif args.command == "continue":
        bench = Bench.carry_on(args.bench)
    else:
        raise ValueError(f"Unsupported command: {args.command}")
    bench.run()
    print('Done.')

if __name__ == '__main__':
    main()
