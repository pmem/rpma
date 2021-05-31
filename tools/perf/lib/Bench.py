#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021, Intel Corporation
#

#
# Bench.py -- control the banchmarking process (EXPERIMENTAL)
#

import sys

from .common import *
from .Benchmark import *
from .Figure import *

class Bench:
    """A benchmarking control object"""

    def __init__(self, config, figures, benchmarks, result_dir):
        self.config = config
        self.figures = figures
        self.benchmarks = benchmarks
        self.result_dir = result_dir

    @classmethod
    def new(cls, config, figures, result_dir):
        figures = Figure.flatten(figures)
        benchmarks = Benchmark.uniq(figures)
        return cls(config, figures, benchmarks, result_dir)

    @classmethod
    def carry_on(cls, bench):
        figures = [Figure(f, bench['result_dir']) for f in bench['figures']]
        benchmarks = {id: Benchmark(b) for id, b in bench['benchmarks'].items()}
        return cls(bench['config'], figures, benchmarks, bench['result_dir'])

    def _cache(self):
        output = {
            'config': self.config,
            'figures': [f.cache() for f in self.figures],
            'benchmarks': {id: b.cache() for id, b in self.benchmarks.items()},
            'result_dir': self.result_dir
        }

        output_path = os.path.join(self.result_dir, 'bench.json')
        with open(output_path, 'w') as file:
            json.dump(output, file, indent=4)

    def run(self):
        self._cache()

        # XXX should be generated from the config
        env = {}

        # run all benchmarks one-by-one
        for _, b in self.benchmarks.items():
            if b.is_done():
                continue
            b.run(env)
            self._cache()

        # collect data required for all scheduled figures
        for f in self.figures:
            if f.is_done():
                continue
            f.prepare_series(self.result_dir)
            self._cache()

    def check_completed(self):
        for _,b in self.benchmarks.items():
            if not b.is_done():
                raise Exception('Benchmarking not completed. Please use report_bench.py.')
        for f in self.figures:
            if not f.is_done():
                raise Exception('Postprocessing not completed. Please use report_bench.py.')
