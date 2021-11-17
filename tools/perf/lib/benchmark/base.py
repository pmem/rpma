#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021, Intel Corporation
#

#
# base.py -- a single benchmark object (EXPERIMENTAL)
#

import json
import os

from ..common import uniq
from .runner import BaseRunner, Bash, Dummy

ENCODE = json.JSONEncoder(indent=4).encode

class Benchmark:
    """A single benchmark object"""

    def __init__(self, oneseries, from_figure=False):
        self.oneseries_from_figure = None
        if from_figure:
            # When oneseries belongs to a Figure:
            # - it should not be modified directly (a copy is required) and
            # - the id when set should be propagated to the original oneseries 
            #   object to allow relating among a unique Benchmark
            #   and potentially multiple series making use of it.
            self.oneseries_from_figure = oneseries
            oneseries = oneseries.copy()
            # validate if it is a mixed workload
            if 'rw' in oneseries and 'rw' in oneseries['rw']:
                if 'rw_dir' not in oneseries:
                    raise SyntaxError(
                        "'rw_dir' is required for mixed workloads (['rw'] == '*rw')\n{}".format(
                            ENCODE(oneseries)))
            # remove unnecessary fields
            oneseries.pop('label', None)
            oneseries.pop('rw_dir', None)
        oneseries['done'] = oneseries.get('done', False)
        self.oneseries = oneseries
        if 'requirements' in oneseries.keys():
            self.req = oneseries['requirements']
            # remove the remaining duplicate
            oneseries.pop('requirements')
        else:
            self.req = {}

    def __repr__(self):
        """A string representation of the object"""
        return ENCODE(self.oneseries)

    def __eq__(self, other):
        """A comparison function"""
        # Benchmark is defined also by its requirements.
        if self.req != other.req:
            return False
        # a complete list of all keys from both objects (without duplicates)
        keys = list(set([*self.oneseries.keys(), *other.oneseries.keys()]))
        for k in keys:
            # ignore series-specific or instance-specific keys
            if k == 'id':
                continue
            sv = self.oneseries.get(k, None)
            ov = other.oneseries.get(k, None)
            if sv != ov:
                return False
        return True

    def set_id(self, id):
        """Set an instance id"""
        self.oneseries['id'] = id
        if self.oneseries_from_figure is not None:
            # Propagated to the original oneseries object to allow relating
            # to the Benchmark object.
            self.oneseries_from_figure['id'] = id

    def get_id(self):
        """Get the instance id"""
        return self.oneseries.get('id', None)

    def get_output_file(self, result_dir):
        """Get the output file path"""
        return os.path.join(result_dir,
            'benchmark_' + str(self.get_id()) + '.json')

    @classmethod
    def uniq(cls, figures):
        """Generate a set of unique benchmarks"""
        output = [cls(oneseries, from_figure=True)
            for f in figures
                for oneseries in f.get_series_in()
        ]
        return uniq(output)

    def get_requirements(self):
        return self.req

    def cache(self):
        """Cache the current state of execution"""
        return self.oneseries

    def is_done(self):
        return self.oneseries['done']

    ONESERIES_REQUIRED = ['filetype', 'id', 'tool', 'mode']
    CONFIG_REQUIRED = ['server_ip']

    def __validate(self, config):
        for required in self.ONESERIES_REQUIRED:
            if required not in self.oneseries:
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
        elif '.sh' in self.oneseries['tool']:
            runner = Bash
        else:
            runner = BaseRunner

        runner.run(self, config, result_dir)
        self.oneseries['done'] = True

    def skip(self):
        self.oneseries['done'] = True
