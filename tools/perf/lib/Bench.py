#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021, Intel Corporation
#

#
# Bench.py -- control the banchmarking process (EXPERIMENTAL)
#

import os
import sys

from .common import *
from .Benchmark import *
from .Figure import *
from .Requirement import *

class Bench:
    """A benchmarking control object"""

    def __init__(self, config, figures, requirements, result_dir):
        # XXX The list of parts has to emerge from the list of provided figures
        # The following quick and dirty WA works only because as of now only
        # a single definition of part/figure exists.
        # Note: The list of parts also has to be stored and restored from
        # the bench.json file.
        self.parts = ['read']
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
        """Cache the current state of execution to a file"""
        output = {
            'config': self.config,
            'figures': [f.cache() for f in self.figures],
            'requirements': {id: r.cache() for id, r in self.requirements.items()},
            'result_dir': self.result_dir
        }

        output_path = os.path.join(self.result_dir, 'bench.json')
        with open(output_path, 'w') as file:
            json.dump(output, file, indent=4)

    def get_config(self):
        return self.config

    def run(self):
        # run all benchmarks one-by-one
        skip = False
        for _, req in self.requirements.items():
            if req.is_done():
                continue
            if not req.is_met(self.config):
                skip = True
                print('Skip: the requirement is not met: ' + str(req))
                continue
            req.benchmarks_run(self, self.result_dir)

        # in case of a skip, not all results are ready
        if skip:
            return

        # collect data required for all scheduled figures
        for f in self.figures:
            if f.is_done():
                continue
            f.prepare_series(self.result_dir)
            self.cache()

    def dump(self):
        for _, req in self.requirements.items():
            if req.is_done():
                status = "done"
            elif req.is_met(self.config):
                status = "met"
            else:
                status = "not met"
            print("Requirement: {} {}\n".format(req, status))
            req.benchmarks_dump(self, self.result_dir)

    def check_completed(self):
        for _, req in self.requirements.items():
            if not req.is_done():
                raise Exception('Benchmarking not completed. Please use report_bench.py.')
        for f in self.figures:
            if not f.is_done():
                raise Exception('Postprocessing not completed. Please use report_bench.py.')
