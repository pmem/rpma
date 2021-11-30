#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021, Intel Corporation
#

#
# base.py
#

"""a single benchmark object (EXPERIMENTAL)"""

import os
from copy import deepcopy

from ..common import uniq, ENCODE
from .runner import BaseRunner, Bash, Dummy

class Benchmark:
    """A single benchmark object"""

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
    def uniq(cls, figures):
        """Generate a set of unique benchmarks"""
        output = [cls(oneseries, figure, index)
            for figure in figures
                for index, oneseries in enumerate(figure.series)
        ]
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
    def identifier(self):
        """Get the instance id"""
        return self.__oneseries.get('id', None)

    @identifier.setter
    def identifier(self, value):
        """Set an instance id"""
        self.__oneseries['id'] = value
        if self.__figure is not None:
            self.__figure.set_series_identifier(self.__series_index, value)

    def get_output_file(self, result_dir):
        """Get the output file path"""
        return os.path.join(result_dir,
            'benchmark_' + str(self.identifier) + '.json')

    @property
    def oneseries(self):
        """XXX"""
        return deepcopy(self.__oneseries)

    @property
    def requirements(self):
        """XXX"""
        return deepcopy(self.__req)

    def cache(self):
        """Cache the current state of execution"""
        return self.__oneseries

    def is_done(self):
        """XXX"""
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

    def run(self, config, result_dir):
        """Run the benchmark and mark it as done.

        Args:
            config (dict): a user-provided system config
            result_dir (str): the directory for the benchmark's results
        Returns:
            None
        """
        self.__validate(config)
        if config.get('dummy_results', False):
            runner = Dummy
        elif '.sh' in self.__oneseries['tool']:
            runner = Bash
        else:
            runner = BaseRunner

        runner.run(self, config, result_dir)
        self.__oneseries['done'] = True

    def skip(self):
        """XXX"""
        self.__oneseries['done'] = True
