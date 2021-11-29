#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021, Intel Corporation
#

"""Bench.py -- control the banchmarking process (EXPERIMENTAL)"""

import json
import os
import re

from copy import deepcopy
from lib.benchmark import Benchmark
from lib.figure import Figure
from lib.Requirement import Requirement
from .remote_cmd import RemoteCmd

class Bench:
    """A benchmarking control object"""

    def __init__(self, config, parts, figures, requirements, result_dir):
        self.__parts = parts
        self.__config = config
        self.__figures = figures
        self.__requirements = requirements
        self.__result_dir = result_dir

    @property
    def parts(self):
        """get a copy of the parts"""
        return self.__parts.copy()

    @property
    def config(self):
        """get a copy of the config"""
        return self.__config.copy()

    @property
    def figures(self):
        """get a copy of the figures"""
        return deepcopy(self.__figures)

    @property
    def requirements(self):
        """get a copy of the requirements"""
        return deepcopy(self.__requirements)

    @property
    def result_dir(self):
        """get the result dir"""
        return self.__result_dir

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
        config = config['json']

        # get cpulist of the remote NUMA node (REMOTE_JOB_NUMA)
        cpulist_path = "/sys/devices/system/node/node" +\
            str(config['REMOTE_JOB_NUMA']) + "/cpulist"
        cpulist = RemoteCmd.run_sync(config, ['cat', cpulist_path], None)
        if cpulist.exit_status != 0:
            raise ValueError(cpulist.stderr.readline())
        cpulist_value = cpulist.stdout.readline().replace('\n', '')
        # validate the output
        if re.match("^[0-9,\-]+$", cpulist_value):
            config['REMOTE_JOB_NUMA_CPULIST'] = cpulist_value
        else:
            raise ValueError("Invalid value REMOTE_JOB_NUMA_CPULIST={}"
                             .format(cpulist_value))

        # get cores per socket
        cmd = "lscpu | egrep 'Core\(s\) per socket:' | sed 's/[^0-9]*//g'"
        cores = RemoteCmd.run_sync(config, cmd, None)
        if cores.exit_status != 0:
            raise ValueError(cpulist.stderr.readline())
        cores_value = cores.stdout.readline().replace('\n', '')
        # validate the output
        if re.match("^[0-9]+$", cores_value):
            config['CORES_PER_SOCKET'] = cores_value
        else:
            raise ValueError("Invalid value CORES_PER_SOCKET={}"
                             .format(cores_value))

        return cls(config, parts, figures, requirements, result_dir)

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
            'config': self.__config,
            'parts': self.__parts,
            'figures': [f.cache() for f in self.__figures],
            'requirements': {
                id: r.cache()
                for id, r in self.__requirements.items()}
        }

        output_path = os.path.join(self.__result_dir, 'bench.json')
        with open(output_path, 'w', encoding='utf-8') as file:
            json.dump(output, file, indent=4)

    def run(self):
        """run all benchmarks one-by-one"""
        skip = False
        for _, req in self.__requirements.items():
            if req.is_done():
                continue
            if self.__config.get('skip_undone', False):
                req.benchmarks_skip(self)
                continue
            # XXX a local copy of config is required as long as
            # Requirement.is_met() modifies the configuration in order to pass
            # some information down to the Bash scripts.
            # No Bash scripts, no modifications to the config, no need to having
            # an altered copy of the config.
            config = self.__config.copy()
            if not req.is_met(config):
                skip = True
                print('Skip: the requirement is not met: ' + str(req))
                continue
            req.benchmarks_run(self, config, self.__result_dir)

        # in case of a skip, not all results are ready
        if skip:
            return

        # collect data required for all scheduled figures
        for figure in self.__figures:
            if figure.is_done():
                continue
            figure.prepare_series(self.__result_dir)
            self.cache()

    def dump(self):
        """print the current status of bench execution"""
        for _, req in self.__requirements.items():
            # XXX a local copy of config is required as long as
            # Requirement.is_met() modifies the configuration in order to pass
            # some information down to the Bash scripts.
            # No Bash scripts, no modifications to the config, no need to having
            # an altered copy of the config.
            config = self.__config.copy()
            if req.is_done():
                status = "done"
            elif req.is_met(config):
                status = "met"
            else:
                status = "not met"
            print("Requirement: {} {}\n".format(req, status))
            req.benchmarks_dump(self, self.__result_dir)

    def check_completed(self):
        """
        if bench is not completed raise an Exception
        (all requirements (benchmarks within them) and all figures)
        """
        for _, req in self.__requirements.items():
            if not req.is_done():
                raise Exception(
                    'Benchmarking not completed. Please use report_bench.py.')
        for figure in self.__figures:
            if not figure.is_done():
                raise Exception(
                    'Postprocessing not completed. Please use report_bench.py.')
