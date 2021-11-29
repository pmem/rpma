#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021, Intel Corporation
#

#
# report_bench.py
#

"""An entry script executing a set of benchmarks (EXPERIMENTAL)

Create or restore the `lib.bench.Bench` object and run all defined benchmarks.

**Note**: A `lib.bench.Bench` object has an internal state which is written down
(cached) into a JSON file (`bench.json`). When the execution is resumed
the content of the file is read and the original `lib.bench.Bench` object is
recreated. For details on this process please see `lib.bench.Bench.carry_on()`.

**Note**: For more details on how `lib.bench.Bench` and other entities come
together, please see `lib`.

  For usage:

  $ ./report_bench.py --help

When this script is done with all required benchmarks you will probably continue
processing the results with `report_figures` and `report_create`.
"""

import argparse

from lib.common import json_from_file, dir_path
from lib.bench import Bench

PARSER = argparse.ArgumentParser(
    description="Executes a set of benchmarks (EXPERIMENTAL)")
PARSER.add_argument('--dump', dest='dump', action='store_true',
                    help='only print a dump')
SUBPARSERS = PARSER.add_subparsers(dest='command')
# Python >= 3.7 accepts 'required' kwarg. For older versions, it is validated
# manually.
SUBPARSERS.required = True
PARSER_R = SUBPARSERS.add_parser('run',
                                 help='start a new benchmarking sequence')
PARSER_R.add_argument('--config', type=json_from_file, required=True,
                      help='''a config.json file describing the configuration of the benchmarking system''')
PARSER_R.add_argument('--figures', type=json_from_file, required=True,
                      help='''a set of JSON files describing what benchmarks are required to be done in a form of figures containing series of data points''',
                      nargs='+')
PARSER_R.add_argument('--result_dir', type=dir_path, required=True,
                      help='''a directory where the intermediate and final products of the benchmarking process will be stored''')
PARSER_R.add_argument('--dummy_results', dest='dummy_results',
                      action='store_true',
                      help='''generate dummy results instead of running actual benchmarks''')
PARSER_C = SUBPARSERS.add_parser('continue', help='''resume not completed benchmarking sequence either due to unmet requirements or in case of an unexpected interruption''')
PARSER_C.add_argument('--bench', type=json_from_file, required=True,
                      help='a bench.json file of an interrupted benchmark')
PARSER_C.add_argument('--skip_undone', dest='skip_undone', action='store_true',
                      help='skip not yet done benchmarks')

def main():
    """I'm main"""
    args = PARSER.parse_args()
    if args.command == "run":
        # modify config according to command line arguments
        args.config['json']['dummy_results'] = args.dummy_results
        bench = Bench.new(args.config, args.figures, args.result_dir)
        bench.cache()
    elif args.command == "continue":
        bench = Bench.carry_on(args.bench, args.skip_undone)
    else:
        raise ValueError('Unsupported command: ' + args.command)
    if args.dump:
        bench.dump()
        print('Dump complete.')
    else:
        if bench.run():
            print('Benchmarking complete.')
        else:
            print('Benchmarking incomplete.')

if __name__ == '__main__':
    main()
