#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021, Intel Corporation
#

#
# Bench.py -- control the banchmarking process (EXPERIMENTAL)
#

import copy
import subprocess
import sys
sys.path.append(".")

from .common import *
from .Figure import *

class Bench:
    """A benchmarking control object"""

    @staticmethod
    def __flatten(figures):
        for f in figures:
            f['output']['done'] = False
            for s in f['series']:
                s['done'] = False
        # - Prepare an empty set B == all benchmarks to be run
        # - Loop over all figure files provided
        #     - Loop over all figures in each of the files
        #         - Loop over all possible combinations of layout and arg_axis
        #             - Generate output_title and output_file (if required)
        #             - Generate filter (if required)
        #             - Loop over all data points defined for the figure
        #                 - Create all possible combinations of the data point keys
        #                 - Loop over all data combinations
        #                     - Populate the B set
        return figures

    @staticmethod
    def __benchmark_hash(figures):
        benchmarks = {}
        for f in figures:
            for s in f['series']:
                benchmarks[s['hash']] = s
        # - Loop over all figures
        #   - Loop over all data points
        #       - Hash benchmark
        #       - Populate benchmarks list
        # - Return benchmarks list
        return benchmarks

    def __init__(self, config, figures, benchmarks, result_dir):
        self.config = config
        self.figures = figures
        self.benchmarks = benchmarks
        self.result_dir = result_dir

    @classmethod
    def new(cls, config, figures, result_dir):
        figures = cls.__flatten(figures)
        return cls(config, figures, cls.__benchmark_hash(figures), result_dir)

    @classmethod
    def carry_on(cls, bench):
        return cls(bench['config'], bench['figures'], bench['benchmarks'], \
            bench['result_dir'])

    def __cache(self):
        output = { \
            'config': self.config, \
            'figures': self.figures, \
            'benchmarks': self.benchmarks, \
            'result_dir': self.result_dir \
        }

        output_path = os.path.join(self.result_dir, 'bench.json')
        with open(output_path, 'w') as file:
            json.dump(output, file, indent=4)
        pass

    def __benchmark_args(self, bench):
        #     - generate a commnd line base on the "b" values
        #     - save results in the hash.json file
        return ['echo', 'XXX'], {}

    def __figure_data_collect(self, fig):
        # verify all required benchmark hash JSON files are in place
        # read required hash JSON files
        # combine data and write to the output file
        pass

    def run(self):
        self.__cache()

        # run all benchmarks one-by-one
        for hash,bench in self.benchmarks.items():
            if bench['done']:
                continue
            args, env = self.__benchmark_args(bench)        
            process = subprocess.run(args, env=env)
            # XXX validate process.returncode and fail with a nice error
            bench['done'] = True
            self.__cache()

        # collect data required for all scheduled figures
        for fig in self.figures:
            if fig['output']['done']:
                continue
            self.__figure_data_collect(fig)
            fig['output']['done'] = True
            self.__cache()

    def check_completed(self):
        for hash, bench in self.benchmarks.items():
            if not bench['done']:
                raise Exception('Benchmarking not completed. Please use report_bench.py.')
        for fig in self.figures:
            if not fig['output']['done']:
                raise Exception('Postprocessing not completed. Please use report_bench.py.')

    def get_figures(self):
        output = []
        for f in self.figures:
            output.append(Figure(f['output'], f['series'], self.result_dir))
        return output
