#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021, Intel Corporation
#

#
# bench.py
#

"""controlling the benchmarking process (EXPERIMENTAL)"""

import json
import os
import re

from copy import deepcopy
from lib.benchmark import Benchmark
from lib.figure import Figure, flatten
from lib.Requirement import Requirement
from .remote_cmd import RemoteCmd

def get_remote_job_numa_cpulist(config):
    """get cpulist of the remote NUMA node (REMOTE_JOB_NUMA)"""
    cpulist_path = "/sys/devices/system/node/node{}/cpulist" \
        .format(str(config['REMOTE_JOB_NUMA']))
    cpulist_cmd = RemoteCmd.run_sync(config, ['cat', cpulist_path],
                                     raise_on_error=True)
    cpulist = cpulist_cmd.stdout.read().decode('utf8').replace('\n', '')
    # validate the output
    if not re.fullmatch(r"^[0-9,\-]+$", cpulist):
        raise ValueError("Invalid value REMOTE_JOB_NUMA_CPULIST={}"
                         .format(cpulist))
    return cpulist

def get_cores_per_socket(config):
    """get cores per socket"""
    cores_cmd = RemoteCmd.run_sync(config, "lscpu", raise_on_error=True)
    output = cores_cmd.stdout.readlines()
    filtered = [line \
        for line in output if re.match(r'Core\(s\) per socket', line)]
    if len(filtered) != 1:
        raise ValueError("Cannot find value CORES_PER_SOCKET\n{}"
                         .format("\n".join(output)))
    cores = re.search('[0-9]+', filtered[0]).group(0)
    # validate the output
    if not cores:
        raise ValueError("Invalid value CORES_PER_SOCKET={}".format(cores))
    return cores

class Bench:
    """A benchmarking control object

    To setup the benchmarking it needs:

    - `Bench.config` - the configuration of the benchmarking system
    - `Bench.parts` - the list of the names of all parts covered (for details
        on parts please see `lib`). To cover a part, the `Bench` object has to
        build all figures contained within the part.
    - `Bench.figures` - the combined list of figures coming from all parts.
        To cover a figure, the `Bench` object has to execute all series
        constituting it. Each of the series has its requirements hence it is
        more efficient to group them by requirements.
    - `Bench.requirements` - the dict of the requirements for all the series
        covered (for details on series please see `lib`). Each of
        the requirements contains a list of the series making the same
        requirement. For the efficiency purposes, the list of series within
        the requirement does not contain duplicates so a single series may be
        used later on in more than one figure.
    - `Bench.result_dir` - a directory where the intermediate and final
        products of the benchmarking process will be stored.

    When the `Bench` object is created its `Bench.run()` method should be
    used to execute the planned series. When it returns `True` all
    the data is collected and it can be further processed for the sake of
    visualisation and publication. Please see `report_figures` and
    `report_create` for details. When `Bench.run()` returns `False`
    the end-user has to pay attention to the standard output in order
    to learn how to adjust the configuration to complete all planned series.
    """

    def __init__(self, config: dict, parts: list,
                 figures: list, requirements: dict,
                 result_dir: str):
        self.__parts = parts
        self.__config = config
        self.__figures = figures
        self.__requirements = requirements
        self.__result_dir = result_dir

    @property
    def parts(self) -> list:
        """(a copy of) the list of the names of all parts covered"""
        return self.__parts.copy()

    @property
    def config(self) -> dict:
        """(a copy of) the configuration of the benchmarking system"""
        return self.__config.copy()

    @property
    def figures(self) -> list:
        """(a copy of) the list of all figures to be covered

        The 'flat' list of all figures comes from all parts covered.
        Being a 'flat' list a single element on this list represents exactly
        one output figure with all series representing exactly one series
        of benchmarks.

        For details on the flattening process please see the `lib.figure.flat`
        module.
        """
        return deepcopy(self.__figures)

    @property
    def requirements(self) -> dict:
        """(a copy of) the dict of the requirements for all the series covered

        Where keys are identifiers of the requirements and the values are
        `lib.Requirement.Requirement` objects.
        """
        return deepcopy(self.__requirements)

    @property
    def result_dir(self) -> str:
        """a directory where the intermediate and final products of the benchmarking process will be stored"""
        return self.__result_dir

    @classmethod
    def new(cls, config: dict, parts: list, result_dir: str) -> 'Bench':
        """combine config and list of figures into a new `Bench` object

        Args:
            config: the configuration of the benchmarking system read from
              the config.json file. `config['json']` is expected to be
              a dictionary providing all required configuration. For details
              about the required and optional parameters please see
              https://github.com/pmem/rpma/blob/master/tools/perf/CONFIG.JSON.md
            parts: a list of parts to be covered. Each of the parts is
              a dictionary. The `part['input_file']` is expected to be the name
              of the source JSON file. The `part['json']` is expected to be
              a list of figures covered by the given part.
            result_dir: a directory where the intermediate and final products
              of the benchmarking process will be stored
        Returns:
            A `Bench` object allowing to run all the ordered benchmarks.
        """
        # combine figures from all the parts
        figures = flatten([
            figure
            for part in parts
            for figure in part['json']], result_dir)
        # generate a list of unique benchmarks to execute
        benchmarks = Benchmark.uniq(figures)
        # generate a list of unique requirements to be met
        requirements = Requirement.uniq(benchmarks)
        # extract names of the parts ABC.json -> ABC
        parts = [os.path.splitext(os.path.basename(part['input_file']))[0]
                 for part in parts]

        config = config['json']
        config['REMOTE_JOB_NUMA_CPULIST'] = get_remote_job_numa_cpulist(config)
        config['CORES_PER_SOCKET'] = get_cores_per_socket(config)

        # create and return a 'Bench' object
        return cls(config, parts, figures, requirements, result_dir)

    @classmethod
    def carry_on(cls, cache: dict, skip_undone=False) -> 'Bench':
        """restore a `Bench` object from the cache

        Args:
            cache: a cache of the `Bench` object. The `cache['input_file']` is
              expected to be the name of the source JSON file.
              The `cache['json]` is expected to be a dictionary in the format
              created by the `Bench.cache()` function.
            skip_undone: instruct the resulting `Bench` object to ignore missing
              series and jump to the end of the benchmarking process.
        """
        # the benchmarking process will continue to use the same directory
        result_dir, _ = os.path.split(os.path.realpath(cache['input_file']))
        cache = cache['json']
        # XXX It might be as well achieved by a simple private variable,
        # no need to modify the config.
        cache['config']['skip_undone'] = skip_undone
        # restore the list of figures
        figures = [Figure(_, result_dir) for _ in cache['figures']]
        # restore the dictionary of requirements
        requirements = {
            id: Requirement(r)
            for id, r in cache['requirements'].items()}
        # recreate and return the 'Bench' object
        return cls(cache['config'], cache['parts'], figures, requirements, \
            result_dir)

    def cache(self) -> None:
        """cache the current state of execution to a JSON file

        A cache is a dictionary with the following keys:

        - `cache['config']` which stores the `Bench.config` property
        - `cache['parts']` which stores the `Bench.parts` property
        - `cache['figures']` which stores the `Bench.figures` property
        - `cache['requirements']` which stores the `Bench.requirements` property

        A cache file is always called `bench.json` and it is created in
        the `Bench.result_dir` directory.

        XXX maybe it is a good idea to rename `bench.json` as `cache.json`.
        """
        # prepare the cache
        output = {
            'config': self.__config,
            'parts': self.__parts,
            'figures': [f.cache() for f in self.__figures],
            'requirements': {
                id: r.cache()
                for id, r in self.__requirements.items()}
        }
        # write the cache down to the JSON file
        output_path = os.path.join(self.__result_dir, 'bench.json')
        with open(output_path, 'w', encoding='utf-8') as file:
            json.dump(output, file, indent=4)

    def run(self) -> bool:
        """run all series one-by-one & collect series into `Bench.figures`

        Browse `Bench.requirements` and if one of them is met
        (`lib.Requirement.Requirement.is_met()`) it executes all series that
        makes the given requirement one-by-one (for details please see
        `lib.Requirement.Requirement.benchmarks_run()`).

        When all series are done, `Bench.figures` are browsed to collect
        all series creating given figures (for details please see
        `lib.figure.base.Figure.collect_results()`).

        Returns:
            `True` if all the benchmarking and postprocessing is complete.
            `False` otherwise.
        """
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
        # in case of a skip, not all series are ready
        if skip:
            return False
        # collect data required for all scheduled figures
        for figure in self.__figures:
            if figure.is_done():
                continue
            figure.collect_results()
            self.cache()
        return True

    def dump(self) -> None:
        """print the current status of the benchmarking execution

        Browse `Bench.requirements` one by one and write down on the screen
        what it exactly requires followed by the list of series demanding
        the requirement to be met (`lib.Requirement.Requirement.is_met()`).
        """
        for req_id, req in self.__requirements.items():
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
            print("Requirement[{}]: {} {}".format(req_id, req, status))
            req.benchmarks_dump()
            print('') # a new line separator

    def check_completed(self) -> None:
        """if bench is not completed raise `Exception`

        Check if all series are complete and the postprocessing is complete.

        Raises:
            Exception: not all benchmarking and postprocessing is complete
        """
        for _, req in self.__requirements.items():
            if not req.is_done():
                raise Exception(
                    'Benchmarking not completed. Please use report_bench.py.')
        for figure in self.__figures:
            if not figure.is_done():
                raise Exception(
                    'Postprocessing not completed. Please use report_bench.py.')
