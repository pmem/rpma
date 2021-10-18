#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021, Intel Corporation
#

"""report_bench.py -- run report-specific benchmarks (EXPERIMENTAL)"""

import argparse

from lib.common import json_from_file, dir_path
from lib.bench import Bench

PARSER = argparse.ArgumentParser(
    description='Run report-specific benchmarks (EXPERIMENTAL)')
PARSER.add_argument('--dump', dest='dump', action='store_true',
                    help='only print a dump')
PARSER.add_argument('--skip_undone', dest='skip_undone', action='store_true',
                    help='skip not yet done benchmarks')
SUBPARSERS = PARSER.add_subparsers(dest='command')
# Python >= 3.7 accepts 'required' kwarg. For older versions, it is validated
# manually.
SUBPARSERS.required = True
PARSER_R = SUBPARSERS.add_parser('run', help='start a new benchmark')
PARSER_R.add_argument('--config', type=json_from_file, required=True,
                      help='a platform configuration file')
PARSER_R.add_argument('--figures', type=json_from_file, required=True,
                      help='figure files describing the benchmark', nargs='+')
PARSER_R.add_argument('--result_dir', type=dir_path, required=True,
                      help='an output directory')
PARSER_R.add_argument('--dummy_results', dest='dummy_results',
                      action='store_true',
                      # XXX allow not breaking long strings
                      help='generate dummy results instead of running ' \
                           'actual benchmarks')
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
        if args.dummy_results:
            args.config['json']['dummy_results'] = True
        if args.skip_undone:
            args.config['json']['skip_undone'] = True
        bench = Bench.new(args.config, args.figures, args.result_dir)
        bench.cache()
    elif args.command == "continue":
        bench = Bench.carry_on(args.bench, args.skip_undone)
    else:
        raise ValueError('Unsupported command: ' + args.command)
    if args.dump:
        bench.dump()
    else:
        bench.run()
        print('Done.')

if __name__ == '__main__':
    main()
