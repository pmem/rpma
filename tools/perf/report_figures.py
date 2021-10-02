#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021, Intel Corporation
#

"""report_figures.py -- generate figures (EXPERIMENTAL)"""

import argparse

from lib.common import dir_path, json_from_file
from lib.compare import Compare
from lib.bench import Bench

PARSER = argparse.ArgumentParser(
    description='Generate figures (EXPERIMENTAL)')
PARSER.add_argument('--report', type=json_from_file,
                    help='a report configuration file')
SUBPARSERS = PARSER.add_subparsers(dest='command')
# Python >= 3.7 accepts 'required' kwarg. For older versions, it is validated
# manually.
SUBPARSERS.required = True
PARSER_R = SUBPARSERS.add_parser('report',
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
                      help='bench.json files of a completed benchmark')
PARSER_C.add_argument('--names', type=str, required=True, nargs='+',
                      help='names for the respective benchmarks')
PARSER_C.add_argument('--result_dir', type=dir_path, required=True,
                      help='an output directory')

def report_figures(args):
    """
    Restore the Bench object, check whether it has completed the benchmarking,
    and loop over all figures to generate their png representations.
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
            figure.prepare_series()
        if figure.argy == 'bw_avg':
            figure.set_yaxis_max(bw_max)
        figure.to_png(args.include_titles)

def comparative_figures(args):
    """generate a comparison figures"""
    benches = [Bench.carry_on(bench) for bench in args.benches]
    compare = Compare(args.names, benches, args.result_dir)
    compare.prepare_series()
    compare.cache()

COMMANDS = {
    "report": report_figures,
    "compare": comparative_figures
}

def main():
    """Pick a command and run it"""
    args = PARSER.parse_args()
    COMMANDS[args.command](args)
    print('Done.')

if __name__ == '__main__':
    main()
