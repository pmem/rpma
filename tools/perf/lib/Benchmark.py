#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021, Intel Corporation
#

#
# Benchmark.py -- a single benchmark object (EXPERIMENTAL)
#

import json
import subprocess

from .common import *

class Benchmark:
    """A single benchmark object"""

    def __init__(self, oneseries):
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
        return json.JSONEncoder(indent=4).encode(self.oneseries)

    def __eq__(self, other):
        """A comparison function"""
        # a complete list of all keys from both objects (without duplicates)
        keys = list(set([*self.oneseries.keys(), *other.oneseries.keys()]))
        for k in keys:
            # ignore series-specific or instance-specific keys
            if k in ['id', 'label']:
                continue
            sv = self.oneseries.get(k, None)
            ov = other.oneseries.get(k, None)
            if sv != ov:
                return False
        return True

    def set_id(self, id):
        """Set an instance id"""
        self.oneseries['id'] = id

    @classmethod
    def uniq(cls, figures):
        """Generate a set of unique benchmarks"""
        output = [cls(oneseries)
            for f in figures
                for oneseries in f.series
        ]
        return uniq(output)

    def get_requirements(self):
        return self.req

    def cache(self):
        """Cache the current state of execution"""
        return self.oneseries

    def is_done(self):
        return self.oneseries['done']

    def _benchmark_args(self):
        # - generate a commnd line base on the "b" values
        # - save results in the benchmark_{id}.json file
        return ['echo', 'XXX']

    def run(self, env):
        args = self._benchmark_args()
        process = subprocess.run(args, env=env)
        process.check_returncode()
        self.oneseries['done'] = True
