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
    figures = bench.figures
    for f in figures:
        f.to_png(bench.result_dir)
        f.to_html(bench.result_dir)
    print('Done.')

if __name__ == '__main__':
    main()
