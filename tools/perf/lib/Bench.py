#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021, Intel Corporation
#

#
# Bench.py -- control the banchmarking process (EXPERIMENTAL)
#

import subprocess
import sys.path

sys.path.append(".")
import common
import Figure

class Bench:
    """A benchmarking control object"""

    def __flatten(self, figures):
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

    def __benchmark_hash(self, figures):
        # - Loop over all figures
        #   - Loop over all data points
        #       - Hash benchmark
        #       - Populate benchmarks list
        # - Return benchmarks list
        return []

    def __init__(self, config, figure):
        self.config = config
        self.figures = flatten(figure) # nop
        self.benchmarks = benchmark_hash(self.figures)

    def __init__(self, bench):
        self.config = bench.config
        self.figures = bench.figures
        self.benchmarks = bench.benchmarks

    def __cache(self):
        # - Write the B set to a disk
        #     - report_XYZ/bench.json 
        #     - this file allows easily continue benchmarking in case of an interupt
        pass

    def __benchmark_args(self, bench):
        #     - generate a commnd line base on the "b" values
        #     - save results in the hash.json file
        return [], {}

    def __figure_data_collect(self, fig):
        # verify all required benchmark hash JSON files are in place
        # read required hash JSON files
        # combine data and write to the output file
        pass

    def run(self):
        self.__cache()

        # run all benchmarks one-by-one
        for bench in self.benchmarks:
            if bench.done:
                continue
            args, env = self.__benchmark_args(bench)        
            process = subprocess.run(args, env=env)
            # XXX validate process.returncode and fail with a nice error
            bench.done = True
            self.__cache()

        # collect data required for all scheduled figures
        for fig in self.figures:
            if fig.done:
                continue
            self.__figure_data_collect(fig)
            fig.done = True
            self.__cache()

    def check_completed(self):
        for bench in self.benchmarks:
            if not bench.done:
                raise Exception("Benchmarking not completed. Please use report_bench.py.")
        for fig in self.figures:
            if not fig.done:
                raise Exception("Postprocessing not completed. Please use report_bench.py.")

    def get_figures(self):
        # - Loop over all figures
        #   - contruct a Figure object from the figure definition and its data
        return []
