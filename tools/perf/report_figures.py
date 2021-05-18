#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021, Intel Corporation
#

#
# report_figures.py -- generate figures (EXPERIMENTAL)
#

import argparse

from lib.common import *
from lib.Bench import *

Parser = argparse.ArgumentParser(
    description='Generate figures (EXPERIMENTAL)')
Parser.add_argument('--bench', type=json_from_file, required=True,
    help='a bench.json file of a completed benchmark')

def main():
    args = Parser.parse_args()
    bench = Bench.carry_on(args.bench)
    bench.check_completed()
    figs = bench.get_figures()
    for fig in figs:
        fig.generate_png()
        fig.generate_html()
    print('Done.')

if __name__ == '__main__':
    main()
