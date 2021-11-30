#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021, Intel Corporation
#

#
# base.py -- a single benchmark object (EXPERIMENTAL)
#

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
                for index, oneseries in enumerate(figure.series_in)
        ]
        return uniq(output)

    def __repr__(self):
        """A string representation of the object"""
        return ENCODE(self.__oneseries)

    def __eq__(self, other):
        """A comparison function"""
        # Benchmark is defined also by its requirements.
        if self.__req != other.__req:
            return False
        # a complete list of all keys from both objects (without duplicates)
        keys = list(set([*self.__oneseries.keys(), *other.__oneseries.keys()]))
        for k in keys:
            # ignore series-specific or instance-specific keys
            if k == 'id':
                continue
            sv = self.__oneseries.get(k, None)
            ov = other.__oneseries.get(k, None)
            if sv != ov:
                return False
        return True

    def set_id(self, id):
        """Set an instance id"""
        self.__oneseries['id'] = id
        if self.__figure is not None:
            self.__figure.set_series_id(self.__series_index, id)

    def get_id(self):
        """Get the instance id"""
        return self.__oneseries.get('id', None)

    def get_output_file(self, result_dir):
        """Get the output file path"""
        return os.path.join(result_dir,
            'benchmark_' + str(self.get_id()) + '.json')

    @property
    def oneseries(self):
        return deepcopy(self.__oneseries)

    def get_requirements(self):
        return deepcopy(self.__req)

    def cache(self):
        """Cache the current state of execution"""
        return self.__oneseries

    def is_done(self):
        return self.__oneseries['done']

    ONESERIES_REQUIRED = ['filetype', 'id', 'tool', 'mode']
    CONFIG_REQUIRED = ['server_ip']

    def __validate(self, config):
        for required in self.ONESERIES_REQUIRED:
            if required not in self.__oneseries:
                raise ValueError("'{}' is missing in the figure".format(required))
        for required in self.CONFIG_REQUIRED:
            if required not in config:
                raise ValueError("'{}' is missing in the config".format(required))

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
        self.__oneseries['done'] = True
