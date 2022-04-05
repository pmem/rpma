#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021-2022, Intel Corporation
#

#
# Requirement.py
#

"""controlling a single requirement (EXPERIMENTAL)

A `Requirement` is a set of conditions that must be satisfied to run
a `lib.benchmark.base.Benchmark` e.g.:

```json
"requirements": {
    "direct_write_to_pmem": true
}
```
"""

import copy
from shutil import which
from os.path import join

from .benchmark import Benchmark
from .common import uniq, ENCODE
from .remote_cmd import RemoteCmd

class Requirement:
    """A single requirement"""

    def __init__(self, req: dict):
        # When we create a new requirement,
        # we need to create a list of benchmarks for it.
        # When we restore the list of requirements, we need to extract
        # the information about the benchmark from the requirements.
        if 'benchmarks' in req.keys():
            benchmarks = req.pop('benchmarks')
            self.__benchmarks = {id: Benchmark(b, req)
                for id, b in benchmarks.items()}
        else:
            self.__benchmarks = {}
        req['done'] = req.get('done', False)
        self.__req = req

    @property
    def identifier(_):
        """Required for compatibility with `lib.common.uniq()`"""
        return None

    @identifier.setter
    def identifier(_self, _):
        pass

    def __repr__(self):
        """A string representation of the object"""
        dup = copy.copy(self.__req)
        dup.pop('done', None)
        dup.pop('benchmarks', None)
        return ENCODE(dup)

    def __eq__(self, other):
        # a complete list of all keys from both objects (without duplicates)
        keys = list(set([*self.__req.keys(), *other.__req.keys()]))
        for k in keys:
            sv = self.__req.get(k, None)
            ov = other.__req.get(k, None)
            if sv != ov:
                return False
        return True

    def __bind(self, benchmarks: dict) -> None:
        """Filter out and bind all benchmarks that make the requirement"""
        self.__benchmarks = {}
        for id, b in benchmarks.items():
            if self == Requirement(b.requirements):
                self.__benchmarks[id] = b

    @classmethod
    def uniq(cls, benchmarks: dict) -> list:
        """Generate a list of unique `Requirement` objects

        A list of `Requirement` objects is created for the given
        `benchmarks`' list (`lib.benchmark.base.Benchmark.requirements`).
        All duplicates are removed from the list. All
        `lib.benchmark.base.Benchmark` objects imposing the `Requirement`
        are bound to it.

        Args:
            benchmarks: A `dict` of `lib.benchmark.base.Benchmark` objects.
              Where keys are `lib.benchmark.base.Benchmark.identifier`.

        Returns:
            A list of `Requirement` objects where each element describes
            different requirement.
        """
        reqs = [cls(b.requirements) for _, b in benchmarks.items()]
        reqs = uniq(reqs)
        for _, req in reqs.items():
            req.__bind(benchmarks)
        return reqs

    def cache(self) -> dict:
        """generate a dict representing the current state of the object

        The state of the object is represented by:

        - a `dict` of requirements and
        - a list of `lib.benchmark.base.Benchmark.cache()` representations
          of the `lib.benchmark.base.Benchmark` objects which makes
          the requirement. The list is stored under `cache['benchmarks']`.

        **Note** this method does not create a separate cache file. It is used
        by the `lib.bench.Bench.cache()` method in order to cache the whole
        state of the execution into a single JSON file.

        Returns:
            A `dict` being a compilation of the current state of the object.
        """
        output = copy.copy(self.__req)
        benchmarks = {}
        for id, b in self.__benchmarks.items():
            benchmarks[id] = b.cache()
        output['benchmarks'] = benchmarks
        return output

    def is_done(self) -> bool:
        """Are all the benchmarks, that make the requirement, done?

        **Note**: `lib.benchmark.base.Benchmark.is_done()` is used to assess
        the state of each of the belonging `lib.benchmark.base.Benchmark`
        objects.

        Returns:
            `True` when all the benchmarks are done. `False` otherwise.
        """
        if self.__req['done']:
            return True
        # it may happen the benchmarks are already done
        for _, b in self.__benchmarks.items():
            if not b.is_done():
                return False
        self.__req['done'] = True
        return True

    def is_met(self, config: dict) -> bool:
        """Is the requirement met?

        **Note**: Depending on the `config['PLATFORM_GENERATION']` it may be
        possible to adjust the system configuration on the fly. Additional
        conditions may apply. For details please see
        https://github.com/pmem/rpma/blob/master/tools/perf/CONFIG.JSON.md.

        Args:
            config: the configuration of the benchmarking system
              (`lib.bench.Bench.config`).

        Returns:
            `True` if the requirement is met. `False` otherwise.

        Raises:
            ValueError: When `config['PLATFORM_GENERATION']` is unknown.
        """
        gen = config['PLATFORM_GENERATION']
        if gen in self.__PLATFORMS.keys():
            # call the generation-specific implementation
            return self.__PLATFORMS[gen].is_met(self.__req, config)
        else:
            raise ValueError("Unsupported 'PLATFORM_GENERATION': '{}'. ".format(gen)
                + "Where supported values are: '"
                + "', '".join(self.__PLATFORMS.keys()) + "'.")

    def benchmarks_run(self, ctx, config: dict, result_dir: str) -> None:
        """Run all belonging benchmarks

        Args:
            ctx: a controlling `lib.bench.Bench` object.
            config: the configuration of the benchmarking system
            result_dir: a directory where the intermediate and final products
              of the benchmarking process will be stored.
        """
        for _, b in self.__benchmarks.items():
            if b.is_done():
                continue
            b.run(config, result_dir)
            ctx.cache() # store to a disk the current state of execution
        self.__req['done'] = True
        ctx.cache() # store to a disk the final state of execution

    def benchmarks_skip(self, ctx):
        """Skip all undone benchmarks

        Please see `lib.benchmark.base.Benchmark.skip()`.

        Args:
            ctx: a controlling `lib.bench.Bench` object.
        """
        for _, b in self.__benchmarks.items():
            if b.is_done():
                continue
            b.skip()
        self.__req['done'] = True
        ctx.cache() # store to a disk the final state of execution

    def benchmarks_dump(self) -> None:
        """Dump all benchmarks

        Benchmark's dump is the string representation of the
        `lib.benchmark.base.Benchmark` object. It aims to allow the human reader
        to understand the internal state of the object. The dump is printed on
        stdout.
        """
        for _, b in self.__benchmarks.items():
            print(b)

    class __CascadeLake:
        """The CLX-specific checks"""

        @classmethod
        def __set_DDIO(cls, req, config):
            """set DDIO to the required value"""
            if config.get('DEBUG_SKIP_RUNNING_TOOLS', False):
                return
            ddio = 'ddio.sh'
            local_dir = '..'
            remote_dir = '/dev/shm'
            local_ddio = join(local_dir, ddio)
            remote_ddio = join(remote_dir, ddio)
            # check if the local copy of ddio.sh exists
            if which(local_ddio) is None:
                raise ValueError('the {} script does not exist!'
                                 .format(local_ddio))

            # copy the ddio.sh script to the remote side
            RemoteCmd.copy_to_remote(config, local_ddio, remote_dir)

            # configure the remote node using ddio.sh via RemoteCmd
            ddio_mode = 'disable' if req['direct_write_to_pmem'] else 'enable'
            ddio_query = 0 if req['direct_write_to_pmem'] else 1
            port = config['REMOTE_RNIC_PCIE_ROOT_PORT']
            # set DDIO on the server
            args = ['sudo', remote_ddio, '-d', port, '-s', ddio_mode]
            rv = RemoteCmd.run_sync(config, args)
            if rv.exit_status != 0:
                raise ValueError('setting DDIO on the remote node failed: {}'
                                 .format(rv.stderr))
            # query DDIO on the server
            args = ['sudo', remote_ddio, '-d', port, '-q']
            rv = RemoteCmd.run_sync(config, args)
            if rv.exit_status != ddio_query:
                raise ValueError('setting DDIO to \'{}\' failed'
                                 .format(ddio_mode))

        @classmethod
        def is_met(cls, req, config):
            # For the CLX generation, it is possible to configure Direct Write
            # to PMem from the OS level.
            if config.get('REMOTE_SUDO_NOPASSWD', False) and \
                    len(config.get('REMOTE_RNIC_PCIE_ROOT_PORT', '')):
                # If there are available: passwordless sudo access and
                # the PCIe Root Port of the RNIC on the remote side
                # the configuration can be adjusted automatically.
                cls.__set_DDIO(req, config)
                return True
            else:
                # Otherwise, the remote Direct Write to PMem configuration
                # has to match the requirement.
                if 'REMOTE_DIRECT_WRITE_TO_PMEM' not in config:
                    raise ValueError('REMOTE_DIRECT_WRITE_TO_PMEM is missing in the config')
                if req['direct_write_to_pmem'] == \
                    config['REMOTE_DIRECT_WRITE_TO_PMEM']:
                    return True
                # the requirement is not met
                if config.get('REMOTE_SUDO_NOPASSWD', False) is False:
                    print('''WARNINIG: REMOTE_SUDO_NOPASSWD is not True in config.json, so DDIO cannot be set!''')
                if len(config.get('REMOTE_RNIC_PCIE_ROOT_PORT', '')) == 0:
                    print('''WARNINIG: REMOTE_RNIC_PCIE_ROOT_PORT is not set in config.json, so DDIO cannot be set!''')
                return False

    class __IceLake:
        """The ICX-specific checks"""

        @classmethod
        def is_met(cls, req, config):
            if 'REMOTE_DIRECT_WRITE_TO_PMEM' not in config:
                raise ValueError('REMOTE_DIRECT_WRITE_TO_PMEM is missing in the config')
            if config.get('xADR', False) is True:
                if config['REMOTE_DIRECT_WRITE_TO_PMEM'] is False:
                    return True
                else:
                    raise ValueError('REMOTE_DIRECT_WRITE_TO_PMEM has to be False when xADR is True')
            # For the ICX generation, there is no way of toggling Direct Write
            # to PMem from the OS level. The configuration has to be adjusted
            # manually on the BIOS level.
            return req['direct_write_to_pmem'] == \
                config['REMOTE_DIRECT_WRITE_TO_PMEM']

    __PLATFORMS = {
        "Cascade Lake": __CascadeLake,
        "Ice Lake": __IceLake
    }
