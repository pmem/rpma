#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021, Intel Corporation
#

"""Bench.py -- control the banchmarking process (EXPERIMENTAL)"""

import json
import os

from lib.benchmark import Benchmark
from lib.figure import Figure
from lib.Requirement import Requirement

class Bench:
    """A benchmarking control object"""

    def __init__(self, config, parts, figures, requirements, result_dir):
        self.parts = parts
        self.config = config
        self.figures = figures
        self.requirements = requirements
        self.result_dir = result_dir

    @classmethod
    def new(cls, config, figures, result_dir):
        """combine config and list of figures into a new bench object"""
        parts = [os.path.splitext(os.path.basename(figure['input_file']))[0]
                 for figure in figures]
        # combine figures from all input files
        figures = Figure.flatten([
            figure
            for figure_file in figures
            for figure in figure_file['json']])
        benchmarks = Benchmark.uniq(figures)
        requirements = Requirement.uniq(benchmarks)
        return cls(config['json'], parts, figures, requirements, result_dir)

    @classmethod
    def carry_on(cls, bench, skip_undone=False):
        """restore a bench object from cache (JSON file)"""
        result_dir, _ = os.path.split(os.path.realpath(bench['input_file']))
        bench = bench['json']
        bench['config']['skip_undone'] = skip_undone
        figures = [Figure(f, result_dir) for f in bench['figures']]
        requirements = {
            id: Requirement(r)
            for id, r in bench['requirements'].items()}
        return cls(bench['config'], bench['parts'], figures, requirements, \
            result_dir)

    def cache(self):
        """cache the current state of execution to a JSON file"""
        output = {
            'config': self.config,
            'parts': self.parts,
            'figures': [f.cache() for f in self.figures],
            'requirements': {
                id: r.cache()
                for id, r in self.requirements.items()}
        }

        output_path = os.path.join(self.result_dir, 'bench.json')
        with open(output_path, 'w', encoding='utf-8') as file:
            json.dump(output, file, indent=4)

    def get_config(self):
        """get the config"""
        return self.config

    def run(self):
        """run all benchmarks one-by-one"""
        skip = False
        for _, req in self.requirements.items():
            if req.is_done():
                continue
            if self.config.get('skip_undone', False):
                req.benchmarks_skip(self)
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
        for figure in self.figures:
            if figure.is_done():
                continue
            figure.prepare_series(self.result_dir)
            self.cache()

    def dump(self):
        """print the current status of bench execution"""
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
        """
        if bench is not completed raise an Exception
        (all requirements (benchmarks within them) and all figures)
        """
        for _, req in self.requirements.items():
            if not req.is_done():
                raise Exception(
                    'Benchmarking not completed. Please use report_bench.py.')
        for figure in self.figures:
            if not figure.is_done():
                raise Exception(
                    'Postprocessing not completed. Please use report_bench.py.')
