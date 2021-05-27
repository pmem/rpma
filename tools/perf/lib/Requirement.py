#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021, Intel Corporation
#

#
# Requirement.py -- a requirements object (EXPERIMENTAL)
#

import copy
import json

from .Benchmark import *

class Requirement:
    """A single set of requirements"""

    def __init__(self, req):
        req['done'] = req.get('done', False)
        self.req = req
        if 'benchmarks' in req.keys():
            self.benchmarks = {id: Benchmark(b)
                for id, b in req['benchmarks'].items()}
        else:
            self.benchmarks = {}

    def __repr__(self):
        """A string representation of the object"""
        dup = copy.copy(self.req)
        dup.pop('done', None)
        dup.pop('benchmarks', None)
        return json.JSONEncoder(indent=4).encode(dup)

    def __eq__(self, other):
        """A comparison function"""
        # a complete list of all keys from both objects (without duplicates)
        keys = list(set([*self.req.keys(), *other.req.keys()]))
        for k in keys:
            sv = self.req.get(k, None)
            ov = other.req.get(k, None)
            if sv != ov:
                return False
        return True

    def set_id(self, id):
        """Required for compatibility with lib.common.uniq()"""
        pass

    def pick(self, benchmarks):
        """Filter out and collect all benchmarks having the requirement"""
        self.benchmarks = {}
        for id, b in benchmarks.items():
            if self == Requirement(b.get_requirements()):
                self.benchmarks[id] = b

    @classmethod
    def uniq(cls, benchmarks):
        """Generate a set of unique requirements"""
        reqs = [cls(b.get_requirements()) for _, b in benchmarks.items()]
        reqs = uniq(reqs)
        for _, req in reqs.items():
            req.pick(benchmarks)
        return reqs

    def cache(self):
        """Cache the current state of execution"""
        output = copy.copy(self.req)
        benchmarks = {}
        for id, b in self.benchmarks.items():
            benchmarks[id] = b.cache()
        output['benchmarks'] = benchmarks
        return output

    def is_done(self):
        if self.req['done']:
            return True
        # it may happen the benchmarks are already done
        for _, b in self.benchmarks.items():
            if not b.is_done():
                return False
        self.req['done'] = True
        return True

    def is_met(self, config):
        """Is the requirement met"""
        # XXX
        return True

    def run_benchmarks(self, ctx, result_dir):
        """Run all benchmarks"""
        for _, b in self.benchmarks.items():
            if b.is_done():
                continue
            b.run(ctx.get_config(), result_dir)
            ctx.cache() # store to a disk the current state of execution
        self.req['done'] = True
        ctx.cache() # store to a disk the final state of execution
