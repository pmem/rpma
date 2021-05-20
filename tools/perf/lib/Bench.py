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
from .Requirement import *

class Bench:
    """A benchmarking control object"""

    def __init__(self, config, figures, requirements, result_dir):
        self.config = config
        self.figures = figures
        self.requirements = requirements
        self.result_dir = result_dir

    @classmethod
    def new(cls, config, figures, result_dir):
        figures = Figure.flatten(figures)
        benchmarks = Benchmark.uniq(figures)
        requirements = Requirement.uniq(benchmarks)
        return cls(config, figures, requirements, result_dir)

    @classmethod
    def carry_on(cls, bench):
        figures = [Figure(f, bench['result_dir']) for f in bench['figures']]
        requirements = {id: Requirement(r) for id, r in bench['requirements'].items()}
        return cls(bench['config'], figures, requirements, bench['result_dir'])

    def cache(self):
        output = {
            'config': self.config,
            'figures': [f.cache() for f in self.figures],
            'requirements': {id: r.cache() for id, r in self.requirements.items()},
            'result_dir': self.result_dir
        }

        output_path = os.path.join(self.result_dir, 'bench.json')
        with open(output_path, 'w') as file:
            json.dump(output, file, indent=4)

    def run(self):
        self.cache()

        # XXX should be generated from the config
        env = {}

        # run all benchmarks one-by-one
        skip = False
        for _, req in self.requirements.items():
            if req.is_done():
                continue
            if not req.is_met(env):
                skip = True
                print('Skip: the requirement is not met: ' + str(req))
                continue
            req.run_benchmarks(env, self)

        # in case of a skip, not all results are ready
        if skip:
            return

        # collect data required for all scheduled figures
        for f in self.figures:
            if f.is_done():
                continue
            f.prepare_series(self.result_dir)
            self.cache()

    def check_completed(self):
        for _,b in self.benchmarks.items():
            if not b.is_done():
                raise Exception('Benchmarking not completed. Please use report_bench.py.')
        for f in self.figures:
            if not f.is_done():
                raise Exception('Postprocessing not completed. Please use report_bench.py.')
