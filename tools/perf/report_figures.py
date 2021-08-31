#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021, Intel Corporation
#

"""report_figures.py -- generate figures (EXPERIMENTAL)"""

import argparse

from lib.common import json_from_file
from lib.bench import Bench

PARSER = argparse.ArgumentParser(
    description='Generate figures (EXPERIMENTAL)')
PARSER.add_argument('--bench', type=json_from_file, required=True,
                    help='a bench.json file of a completed benchmark')
PARSER.add_argument('--include_titles', dest='include_titles',
                    action='store_true',
                    help='include titles into the output figures')

def main():
    """
    Restore the Bench object, check whether it has completed the benchmarking,
    and loop over all figures to generate their png representations.
    """
    args = PARSER.parse_args()
    bench = Bench.carry_on(args.bench)
    bench.check_completed()
    for figure in bench.figures:
        yaxis_max = 0
        if 'rnic_max_bw' in bench.config:
            yaxis_max = 1.1 * bench.config['rnic_max_bw']
        figure.to_png(args.include_titles, yaxis_max)
    print('Done.')

if __name__ == '__main__':
    main()
