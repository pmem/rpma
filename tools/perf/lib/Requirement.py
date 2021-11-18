#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021, Intel Corporation
#

#
# Requirement.py -- a requirements objects (EXPERIMENTAL)
#

import copy
import json

from .benchmark import Benchmark
from .common import uniq

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
        gen = config['platform_generation']
        if gen in self.PLATFORMS.keys():
            # call the generation-specific implementation
            return self.PLATFORMS[gen].is_met(self.req, config)
        else:
            raise ValueError("Unsupported 'platform_generation': '{}'. ".format(gen)
                + "Where supported values are: '"
                + "', '".join(self.PLATFORMS.keys()) + "'.")

    def benchmarks_run(self, ctx, result_dir):
        """Run all benchmarks"""
        for _, b in self.benchmarks.items():
            if b.is_done():
                continue
            b.run(ctx.get_config(), result_dir)
            ctx.cache() # store to a disk the current state of execution
        self.req['done'] = True
        ctx.cache() # store to a disk the final state of execution

    def benchmarks_skip(self, ctx):
        """Skip all undone benchmarks"""
        for _, b in self.benchmarks.items():
            if b.is_done():
                continue
            b.skip()
        self.req['done'] = True
        ctx.cache() # store to a disk the final state of execution

    def benchmarks_dump(self, ctx, result_dir):
        """Dump all benchmarks"""
        for _, b in self.benchmarks.items():
            b.dump(ctx.get_config(), result_dir)
            print('') # a new line separator

    class CascadeLake:
        """The CLX-specific checks"""

        @classmethod
        def __set_DDIO(req, config):
            # XXX check if the local copy of ddio.sh exists
            # XXX copy the ddio.sh script to the remote side
            # XXX configure the remote node using ddio.sh via RemoteCmd
            raise NotImplementedError()

        @classmethod
        def is_met(req, config):
            # For the CLX generation, it is possible to configure Direct Write
            # to PMem from the OS level.
            if config.get('REMOTE_SUDO_NOPASSWD', False) and \
                    len(config.get('REMOTE_RNIC_PCIE_ROOT_PORT', '')):
                # If there are available: passwordless sudo access and
                # the PCIe Root Port of the RNIC on the remote side
                # the configuration can be adjusted automatically.
                # XXX remove when Bash scripts will be removed
                config['FORCE_REMOTE_DIRECT_WRITE_TO_PMEM'] = \
                    req['direct_write_to_pmem']
                # XXX for the non-Bash runners, it is the only viable implementation
                # so a dedicated variable won't be needed when the Bash runner will
                # be decommissioned.
                if config['THE_NEW_DDIO_IMPL']:
                    cls.__set_DDIO(req, config)
                return True
            else:
                # Otherwise, the remote Direct Write to PMem configuration
                # has to match the requirement.
                return req['direct_write_to_pmem'] == \
                    config['REMOTE_DIRECT_WRITE_TO_PMEM']

    class IceLake:
        """The ICX-specific checks"""

        @classmethod
        def is_met(req, config):
            # For the ICX generation, there is no way of toggling Direct Write
            # to PMem from the OS level. The configuration has to be adjusted
            # manually on the BIOS level.
            return req['direct_write_to_pmem'] == \
                config['REMOTE_DIRECT_WRITE_TO_PMEM']

    PLATFORMS = {
        "Cascade Lake": CascadeLake,
        "Ice Lake": IceLake
    }
