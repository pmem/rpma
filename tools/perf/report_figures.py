#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021-2022, Intel Corporation
#

#
# report_figures.py
#

"""A script generating images from data collected by the `report_bench`
(EXPERIMENTAL)

**Note**: A `lib.bench.Bench` object has an internal state which is written down
(cached) into a JSON file (`bench.json`). When the execution is resumed
the content of the file is read and the original `lib.bench.Bench` object is
recreated. For details on this process please see `lib.bench.Bench.carry_on()`.

**Note**: For more details on how `lib.bench.Bench` and other entities come
together, please see `lib`.

  For usage:

  $ ./report_figures.py --help

When this script is done you probably will continue processing the results with
`report_create`.
"""

import argparse

from lib.common import dir_path, json_from_file
from lib.compare import Compare
from lib.bench import Bench

PARSER = argparse.ArgumentParser(
    description='Generate figures (EXPERIMENTAL)')
PARSER.add_argument('--report', type=json_from_file,
                    help='''a report.json configuration file configures the visualization process''')
SUBPARSERS = PARSER.add_subparsers(dest='command')
# Python >= 3.7 accepts 'required' kwarg. For older versions, it is validated
# manually.
SUBPARSERS.required = True
PARSER_R = SUBPARSERS.add_parser('generate',
                                 help='generate figures for the report')
PARSER_R.add_argument('--bench', type=json_from_file, required=True,
                      help='a bench.json file of a completed benchmark')
PARSER_R.add_argument('--regenerate_jsons', dest='regenerate_jsons',
                      action='store_true',
                      help='regenerate file.json files from benchmark_*.json')
PARSER_R.add_argument('--include_titles', dest='include_titles',
                      action='store_true',
                      help='include titles into the output figures')
PARSER_C = SUBPARSERS.add_parser('compare', help='generate comparative figures')
PARSER_C.add_argument('--benches', type=json_from_file, required=True,
                      nargs='+',
                      help='bench.json files of completed benchmarks')
PARSER_C.add_argument('--prefixes', type=str, required=True, nargs='+',
                      help='''prefixes to be added to the series names of the respective benchmarks''')
PARSER_C.add_argument('--result_dir', type=dir_path, required=True,
                      help='an output directory')

def generate_figures(args: argparse.Namespace) -> None:
    """Restore the `lib.bench.Bench` object, check whether it has completed
    the ordered benchmarks.

    XXXX Loop over all figures to generate their png representations.
    """
    bench = Bench.carry_on(args.bench)
    bench.check_completed()
    bw_max = None
    if args.report:
        bw_max = args.report['json'].get('visualization', {}).\
            get('rnic_max_bw', None)
    if bw_max is not None:
        bw_max *= 1.1
    for figure in bench.figures:
        if args.regenerate_jsons:
            figure.collect_results()
        if figure.argy == 'bw_avg':
            figure.yaxis_max = bw_max
        figure.to_png(args.include_titles)

def comparative_figures(args: argparse.Namespace) -> None:
    """Restore `lib.bench.Bench` objects. Create an intermedia `lib.compare.Compare` object and generate JPEG and JSON files comparing all given `lib.bench.Bench` objects."""
    benches = [Bench.carry_on(bench) for bench in args.benches]
    compare = Compare(args.prefixes, benches, args.result_dir)
    compare.prepare_series()
    compare.cache()
    compare.to_html()

COMMANDS = {
    "generate": generate_figures,
    "compare": comparative_figures
}

def main():
    """Pick a command and run it"""
    args = PARSER.parse_args()
    COMMANDS[args.command](args)
    print('Done.')

if __name__ == '__main__':
    main()
