#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021, Intel Corporation
#

#
# base.py
#

"""controlling a single benchmark (EXPERIMENTAL)"""

import os

from copy import deepcopy

from ..common import uniq, ENCODE
from .runner import BaseRunner, Bash, Dummy

def get_result_path(result_dir: str, identifier: int) -> str:
    """a path to the file with all the collected results of the particular
    benchmark

    Args:
        result_dir: a directory where the intermediate and final products
          of the benchmarking process will be stored.

        identifier: an identifier of `lib.benchmark.base.Benchmark` you
          are looking for.

    Returns:
        The path to the file with all the collected results.
    """
    return os.path.join(result_dir, 'benchmark_{}.json'.format(str(identifier)))

class Benchmark:
    """A single benchmark

    **Note**: A single benchmark may cover a single series executed for
    one or more `lib.figure.base.Figure` objects. The `Benchmark` covers a
    series if `Benchmark.identifier` == `series['id']`. Where `series` is
    an element of a `lib.figure.base.Figure.series` list.
    """

    def __init__(self, oneseries, figure=None, series_index=-1):
        self.__figure = figure
        self.__series_index = series_index
        # remove unnecessary fields
        oneseries.pop('label', None)
        oneseries.pop('rw_dir', None)
        oneseries['done'] = oneseries.get('done', False)
        self.__req = oneseries.pop('requirements', {})
        self.__oneseries = oneseries

    @classmethod
    def uniq(cls, figures: list) -> list:
        """Generate a set of unique benchmarks

        `figures` provide `lib.figure.base.Figure.series` which are used to
        create a list of `Benchmark` objects. From this list are removed all
        duplicates. The remaining objects have assigned unique
        `Benchmark.identifier` values which are propagated to
        `lib.figure.base.Figure.series` via
        `lib.figure.base.Figure.set_series_identifier()`.

        **Note**: Two `Benchmark` objects are different when either do things
        differently or make different requirements.

        Args:
            figures: A list of `lib.figure.base.Figure` objects.

        Returns:
            A list of `Benchmark` objects where each element describes
            different benchmark.
        """
        output = [cls(oneseries, figure, index)
                  for figure in figures
                  for index, oneseries in enumerate(figure.series)]
        return uniq(output)

    def __repr__(self):
        """A string representation of the object"""
        return ENCODE(self.__oneseries)

    def __eq__(self, other):
        """A comparison function"""
        # Benchmark is defined also by its requirements.
        if self.requirements != other.requirements:
            return False
        # a complete list of all keys from both objects (without duplicates)
        keys = list(set([*self.oneseries.keys(), *other.oneseries.keys()]))
        for k in keys:
            # ignore series-specific or instance-specific keys
            if k == 'id':
                continue
            self_value = self.oneseries.get(k, None)
            other_value = other.oneseries.get(k, None)
            if self_value != other_value:
                return False
        return True

    @property
    def identifier(self) -> int:
        """a unique identifier of the instance

        For details please see `Benchmark.uniq()`.
        """
        return self.__oneseries.get('id', None)

    @identifier.setter
    def identifier(self, value):
        self.__oneseries['id'] = value
        if self.__figure is not None:
            self.__figure.set_series_identifier(self.__series_index, value)

    @property
    def oneseries(self) -> dict:
        """(a copy of) a description of what and how run the benchmark"""
        return deepcopy(self.__oneseries)

    @property
    def requirements(self) -> dict:
        """(a copy of) a description of what requirements have to be met before
        the `Benchmark` can be started

        It is used to create an instance of `lib.Requirement.Requirement`
        taking care of all `Benchmark` objects making this requirement.
        """
        return deepcopy(self.__req)

    def cache(self) -> dict:
        """generate a dict representing the current state of the object

        The state of the object is fully represented by
        the `Benchmark.oneseries` property.

        **Note** this method does not create a separate cache file. It is used
        by the `lib.bench.Bench.cache()` method in order to cache the whole
        state of the execution into a single JSON file.

        Returns:
            A `dict` being a compilation of the current state of the benchmark.
        """
        return self.__oneseries

    def is_done(self) -> bool:
        """Have all the benchmark's results been collected?

        Returns:
            `True` when all the results have been already collected. `False`
            otherwise.
        """
        return self.__oneseries['done']

    __ONESERIES_REQUIRED = ['filetype', 'id', 'tool', 'mode']
    __CONFIG_REQUIRED = ['server_ip']

    def __validate(self, config):
        for required in self.__ONESERIES_REQUIRED:
            if required not in self.__oneseries:
                raise ValueError(
                    "'{}' is missing in the figure".format(required))
        for required in self.__CONFIG_REQUIRED:
            if required not in config:
                raise ValueError(
                    "'{}' is missing in the config".format(required))

    def run(self, config: dict, result_dir: str) -> None:
        """Pick a runner and run the benchmark.

        Either:

        - `lib.benchmark.runner.dummy.Dummy.run()` when not the actual results
          are expected,
        - `lib.benchmark.runner.bash.Bash.run()` when the legacy `ib_read.sh`
          and `rpma_fio_bench.sh` are meant to be used or
        - `lib.benchmark.runner.base.BaseRunner.run()`.

        No matter which runner is used, in the result of successful
        execution the `Benchmark` is marked as done (`Benchmark.is_done()`)
        and the benchmark's result file (`get_result_path()`) contains all
        the collected results.

        Args:
            config: the configuration of the benchmarking system

            result_dir: a directory where the intermediate and final products
             of the benchmarking process will be stored.
        """
        self.__validate(config)
        if config.get('dummy_results', False):
            runner = Dummy
        elif '.sh' in self.__oneseries['tool']:
            runner = Bash
        else:
            runner = BaseRunner

        idfile = get_result_path(result_dir, self.identifier)
        runner.run(self, config, idfile)
        self.__oneseries['done'] = True

    def skip(self) -> None:
        """Mark the benchmark as done."""
        self.__oneseries['done'] = True
